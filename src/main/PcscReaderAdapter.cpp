/******************************************************************************
 * Copyright (c) 2025 Calypso Networks Association https://calypsonet.org/    *
 *                                                                            *
 * See the NOTICE file(s) distributed with this work for additional           *
 * information regarding copyright ownership.                                 *
 *                                                                            *
 * This program and the accompanying materials are made available under the   *
 * terms of the Eclipse Public License 2.0 which is available at              *
 * http://www.eclipse.org/legal/epl-2.0                                       *
 *                                                                            *
 * SPDX-License-Identifier: EPL-2.0                                           *
 ******************************************************************************/

#include "keyple/plugin/pcsc/PcscReaderAdapter.hpp"

#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
#include <winscard.h>
#else
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#endif

#include "keyple/core/plugin/CardIOException.hpp"
#include "keyple/core/plugin/ReaderIOException.hpp"
#include "keyple/core/plugin/TaskCanceledException.hpp"
#include "keyple/core/util/HexUtil.hpp"
#include "keyple/core/util/cpp/Thread.hpp"
#include "keyple/core/util/cpp/exception/Exception.hpp"
#include "keyple/core/util/cpp/exception/IllegalArgumentException.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/core/util/cpp/exception/InterruptedException.hpp"
#include "keyple/plugin/pcsc/PcscPluginAdapter.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardException.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardNotPresentException.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::plugin::CardIOException;
using keyple::core::plugin::ReaderIOException;
using keyple::core::plugin::TaskCanceledException;
using keyple::core::util::HexUtil;
using keyple::core::util::cpp::Thread;
using keyple::core::util::cpp::exception::Exception;
using keyple::core::util::cpp::exception::IllegalArgumentException;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::core::util::cpp::exception::InterruptedException;
using keyple::plugin::pcsc::cpp::exception::CardException;
using keyple::plugin::pcsc::cpp::exception::CardNotPresentException;

PcscReaderAdapter::PcscReaderAdapter(
    std::shared_ptr<CardTerminal> terminal,
    std::shared_ptr<PcscPluginAdapter> pluginAdapter,
    const int cardMonitoringCycleDuration)
: mIsInitialized(false)
, mIsPhysicalChannelOpen(false)
, mTerminal(terminal)
, mName(terminal->getName())
, mPluginAdapter(pluginAdapter)
, mCardMonitoringCycleDuration(cardMonitoringCycleDuration)
, mPingApdu(HexUtil::toByteArray("00C0000000")) // GET RESPONSE
, mIsContactless(false)
, mProtocol(IsoProtocol::ANY.getValue())
, mIsModeExclusive(false)
, mDisconnectionMode(keyple::plugin::pcsc::PcscReader::DisconnectionMode::RESET)
, mLoopWaitCard(false)
, mLoopWaitCardRemoval(false)
, mIsObservationActive(false)
{
#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
    mIsWindows = true;
#else
    mIsWindows = false;
#endif
}

void
PcscReaderAdapter::waitForCardInsertion()
{
    mLogger->trace(
        "Reader [%]: start waiting card insertion (loop latency: % ms)\n",
        getName(),
        mCardMonitoringCycleDuration);

    /* Activate loop */
    mLoopWaitCard = true;

    try {
        while (mLoopWaitCard) {
            if (mTerminal->waitForCardPresent(mCardMonitoringCycleDuration)) {
                /* Card inserted */
                mLogger->trace("Reader [%]: card inserted\n", getName());
                return;
            }

            // if (Thread.interrupted()) {
            //     break;
            // }
        }

        mLogger->trace(
            "Reader [%]: waiting card insertion stopped\n", getName());

    } catch (const CardException& e) {
        /* Here, it is a communication failure with the reader */
        throw ReaderIOException(
            mName + ": an error occurred while waiting for a card insertion.",
            std::make_shared<CardException>(e));
    }

    throw TaskCanceledException(
        mName + ": the wait for a card insertion task has been cancelled.");
}

void
PcscReaderAdapter::stopWaitForCardInsertion()
{
    mLoopWaitCard = false;
}

bool
PcscReaderAdapter::isProtocolSupported(const std::string& readerProtocol) const
{
    /* C++ specific: getProtocolRule throws when protocol not found instead of
     * returning null */
    try {
        const std::string rulmIsPhysicalChannelOpen
            = mPluginAdapter->getProtocolRule(readerProtocol);

        return true;

    } catch (const IllegalArgumentException&) {
        /* Do nothing */
    }

    return false;
}

void
PcscReaderAdapter::activateProtocol(const std::string& readerProtocol)
{
    mLogger->trace(
        "%: stop waiting for card insertion requested.\n", getName());
    mLogger->trace(
        "Reader [%]: activating protocol [%] takes no action\n",
        getName(),
        readerProtocol);

    mLoopWaitCard = false;
}

void
PcscReaderAdapter::deactivateProtocol(const std::string& readerProtocol)
{
    mLogger->trace(
        "Reader [%]: de-activating protocol [%] takes no action\n",
        getName(),
        readerProtocol);
}

bool
PcscReaderAdapter::isCurrentProtocol(const std::string& readerProtocol) const
{
    /*
     * C++ specific: getProtocolRule throws when protocol not found instead of
     * returning null.
     */
    try {
        bool isCurrentProtocol = false;

        const std::string protocolRule
            = mPluginAdapter->getProtocolRule(readerProtocol);
        if (!protocolRule.empty()) {
            const std::string atr = HexUtil::toHex(mCard->getATR());
            isCurrentProtocol
                = Pattern::compile(protocolRule)->matcher(atr)->matches();
        }

        return isCurrentProtocol;

    } catch (const IllegalArgumentException&) {
        /* Do nothing */
    }

    return false;
}

void
PcscReaderAdapter::onStartDetection()
{
    mIsObservationActive = true;
}

void
PcscReaderAdapter::onStopDetection()
{
    mIsObservationActive = false;
}

const std::string&
PcscReaderAdapter::getName() const
{
    return mName;
}

void
PcscReaderAdapter::openPhysicalChannel()
{
    if (mCard != nullptr) {
        return;
    }

    /*
     * Init of the card physical channel: if not yet established, opening of a
     * new physical channel.
     */
    try {
        mLogger->debug(
            "Reader [%]: open card physical channel for protocol [%]\n",
            getName(),
            mProtocol);

        mCard = mTerminal->connect(mProtocol);
        if (mIsModeExclusive) {
            mCard->beginExclusive();
            mLogger->debug(
                "Reader [%]: open card physical channel in exclusive mode\n",
                getName());

        } else {
            mLogger->debug(
                "%: opening of a card physical channel in shared mode\n",
                getName());
        }

        mChannel = mCard->getBasicChannel();

    } catch (const CardNotPresentException& e) {
        throw CardIOException(
            "Card removed", std::make_shared<CardNotPresentException>(e));

    } catch (const CardException& e) {
        throw ReaderIOException(
            getName() + ": Error while opening Physical Channel",
            std::make_shared<CardException>(e));
    }
}

void
PcscReaderAdapter::closePhysicalChannel()
{
    /*
     * If the reader is observed, the actual disconnection will be done in the
     * card removal sequence.
     */
    if (!mIsObservationActive) {
        disconnect();
    }
}

void PcscReaderAdapter::disconnect()
{
    try {
        if (mCard != nullptr) {
            /* Disconnect using the extended mode allowing UNPOWER. */
            mCard->disconnect(getDisposition(mDisconnectionMode));
            /* Reset the reader state to avoid bad card detection next time. */
            resetReaderState();
        }

    } catch (const CardNotPresentException& e) {
        throw CardIOException(
            "Card removed", std::make_shared<CardNotPresentException>(e));

    } catch (const CardException& e) {
        throw ReaderIOException(
            "Error while closing physical channel",
            std::make_shared<CardException>(e));
    }

    resetContext();
}

int PcscReaderAdapter::getDisposition(const DisconnectionMode mode)
{
    switch (mode) {
    case DisconnectionMode::RESET:
        return SCARD_RESET_CARD;
    case DisconnectionMode::LEAVE:
        return SCARD_LEAVE_CARD;
    case DisconnectionMode::UNPOWER:
        return SCARD_UNPOWER_CARD;
    case DisconnectionMode::EJECT:
        return SCARD_EJECT_CARD;
    default:
        throw IllegalArgumentException(std::string("Unknown DisconnectionMode: ") + mode);
    }
}

void PcscReaderAdapter::resetReaderState()
{
    try {
        if (mDisconnectionMode == DisconnectionMode::UNPOWER) {
            mTerminal->connect("*")->disconnect(false);
        }

    } catch (const CardException& /*e*/) {
        /* NOP */
    }
}

bool
PcscReaderAdapter::isPhysicalChannelOpen() const
{
    return mIsPhysicalChannelOpen;
}

bool
PcscReaderAdapter::checkCardPresence()
{
    try {
        const bool isCardPresent = mTerminal->isCardPresent();
        closePhysicalChannelSafely();

        return isCardPresent;

    } catch (const CardException& e) {
        throw ReaderIOException(
            "Exception occurred in isCardPresent",
            std::make_shared<CardException>(e));
    }
}

void
PcscReaderAdapter::closePhysicalChannelSafely()
{
    try {
        disconnect();

    } catch (const Exception&) {
        /* NOP */
    }
}

void
PcscReaderAdapter::resetContext()
{
    mCard = nullptr;
    mIsPhysicalChannelOpen = false;
}

const std::string
PcscReaderAdapter::getPowerOnData() const
{
    return HexUtil::toHex(mCard->getATR());
}

const std::vector<uint8_t>
PcscReaderAdapter::transmitApdu(const std::vector<uint8_t>& apduCommandData)
{
    std::vector<uint8_t> apduResponseData;

    if (mChannel) {
        try {
            apduResponseData = mChannel->transmit(apduCommandData);

        } catch (const CardNotPresentException& e) {
            throw CardIOException(
                mName + ": " + e.getMessage(),
                std::make_shared<CardNotPresentException>(e));

        } catch (const CardException& e) {
            if (e.getMessage().find("CARD") != std::string::npos ||
                e.getMessage().find("NOT_TRANSACTED") != std::string::npos ||
                e.getMessage().find("INVALID_ATR") != std::string::npos) {
                throw CardIOException(
                    getName() + ":" + e.getMessage(),
                    std::make_shared<CardException>(e));
            } else {
                throw ReaderIOException(
                    getName() + ":" + e.getMessage(),
                    std::make_shared<CardException>(e));
            }

        } catch (const IllegalStateException& e) {
            /* Card could have been removed prematurely */
            throw CardIOException(
                getName() + ":" + e.getMessage(),
                std::make_shared<IllegalStateException>(e.getMessage()));

        } catch (const IllegalArgumentException& e) {
            /* Card could have been removed prematurely */
            throw CardIOException(
                getName() + ":" + e.getMessage(),
                std::make_shared<IllegalStateException>(e.getMessage()));
        }

    } else {
        /* Could occur if the card was removed */
        throw CardIOException(getName() + ": null channel.");
    }

    return apduResponseData;
}

bool
PcscReaderAdapter::isContactless()
{
    if (!mIsInitialized) {
        /*
         * First time initialisation, the transmission mode has not yet been
         * determined or fixed explicitly, let's ask the plugin to determine it
         * (only once)
         */
        mIsContactless = mPluginAdapter->isContactless(getName());
    }

    return mIsContactless;
}

void
PcscReaderAdapter::onUnregister()
{
    /* Nothing to do here in this reader */
}

void
PcscReaderAdapter::monitorCardPresenceDuringProcessing()
{
    waitForCardRemoval();
}

void
PcscReaderAdapter::stopCardPresenceMonitoringDuringProcessing()
{
    stopWaitForCardRemoval();
}

void
PcscReaderAdapter::waitForCardRemoval()
{
    mLogger->trace("Reader [%]: start waiting card removal\n", mName);

    mLoopWaitCardRemoval = true;

    try {
        if (mDisconnectionMode == DisconnectionMode::UNPOWER) {
            waitForCardRemovalByPolling();
        } else {
            waitForCardRemovalStandard();
        }

    } catch (const Exception&) {
    }

    /* Finally */
    try {
        disconnect();

    } catch (const Exception& e) {
        mLogger->warn(
            "Error while disconnecting card during card removal: %\n",
            e.getMessage());
    }


    if (!mLoopWaitCardRemoval) {
        mLogger->trace("Reader [%]: waiting card removal stopped\n", mName);
    } else {
        mLogger->trace("Reader [%]: card removed\n", mName);
    }

    if (!mLoopWaitCardRemoval) {
        throw TaskCanceledException(
          mName + ": the wait for the card removal task has been cancelled.");
    }
}

void
PcscReaderAdapter::waitForCardRemovalByPolling()
{
    try {
        while (mLoopWaitCardRemoval) {
            transmitApdu(mPingApdu);
            Thread::sleep(25);
            // if (Thread::isInterrupted()) {
            //     return;
            // }
        }

    } catch (const CardIOException& e) {
        mLogger->trace(
            "Expected IOException while waiting for card removal: %\n",
            e.getMessage());

    } catch (const ReaderIOException& e) {
        mLogger->trace(
            "Expected IOException while waiting for card removal: %\n",
            e.getMessage());

    } catch (const InterruptedException& e) {
        mLogger->trace(
            "InterruptedException while waiting for card removal: %\n",
            e.getMessage());
        // Thread::currentThread().interrupt();
    }
  }

void PcscReaderAdapter::waitForCardRemovalStandard()
{
    try {
        while (mLoopWaitCardRemoval) {
            if (mTerminal->waitForCardAbsent(mCardMonitoringCycleDuration)) {
                return;
            }
            // if (isInterrupted()) {
            //     return;
            // }
        }

    } catch (const CardException& e) {
        mLogger->trace(
            "Expected CardException while waiting for card removal: %\n",
            e.getMessage());

        throw ReaderIOException(
            mName + ": an error occurred while waiting for the card removal.",
            std::make_shared<CardException>(e));
    }
}

void
PcscReaderAdapter::stopWaitForCardRemoval()
{
    mLoopWaitCardRemoval = false;
}

PcscReader&
PcscReaderAdapter::setSharingMode(const SharingMode sharingMode)
{
    mLogger->trace(
        "Reader [%]: set sharing mode to [%]\n", getName(), sharingMode);

    if (sharingMode == SharingMode::SHARED) {
        /* If a card is present, change the mode immediately */
        if (mCard != nullptr) {
            try {
                mCard->endExclusive();

            } catch (const CardException& e) {
                throw IllegalStateException(
                    "Couldn't disable exclusive mode",
                    std::make_shared<CardException>(e));
            }
        }

        mIsModeExclusive = false;

    } else if (sharingMode == SharingMode::EXCLUSIVE) {
        mIsModeExclusive = true;
    }

    return *this;
}

PcscReader&
PcscReaderAdapter::setContactless(const bool contactless)
{
    mLogger->trace(
        "Reader [%]: set contactless type to [%]\n", getName(), contactless);

    mIsContactless = contactless;
    mIsInitialized = true;

    return *this;
}

PcscReader&
PcscReaderAdapter::setIsoProtocol(const IsoProtocol& isoProtocol)
{
    mLogger->trace(
        "Reader [%]: set ISO protocol to [%] (%)\n",
        getName(),
        isoProtocol,
        isoProtocol.getValue());

    mProtocol = isoProtocol.getValue();

    return *this;
}

PcscReader&
PcscReaderAdapter::setDisconnectionMode(
    const DisconnectionMode disconnectionMode)
{
    mLogger->trace(
        "Reader [%]: set disconnection mode to [%]\n",
        getName(),
        disconnectionMode);

    mDisconnectionMode = disconnectionMode;

    return *this;
}

const std::vector<uint8_t>
PcscReaderAdapter::transmitControlCommand(
    const int commandId, const std::vector<uint8_t>& command)
{
    std::vector<uint8_t> response;
    const int controlCode
        = mIsWindows ? 0x00310000 | (commandId << 2) : 0x42000000 | commandId;

    try {
        if (mCard != nullptr) {

            response = mCard->transmitControlCommand(controlCode, command);
        } else {
            std::shared_ptr<Card> virtualCard = mTerminal->connect("DIRECT");
            response = virtualCard->transmitControlCommand(controlCode, command);
            virtualCard->disconnect(false);
        }

    } catch (const CardException& e) {
        throw IllegalStateException(
            "Reader failure.", std::make_shared<CardException>(e));
    }

    return response;
}

int
PcscReaderAdapter::getIoctlCcidEscapeCommandId() const
{
    return mIsWindows ? 3500 : 1;
}

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
