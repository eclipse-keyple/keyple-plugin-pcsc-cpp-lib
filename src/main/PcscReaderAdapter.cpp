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
#include "keyple/plugin/pcsc/PcscCardCommunicationProtocol.hpp"
#include "keyple/plugin/pcsc/PcscPluginAdapter.hpp"
#include "keyple/plugin/pcsc/cpp/CardTerminals.hpp"
#include "keyple/plugin/pcsc/cpp/TerminalFactory.hpp"
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
using keyple::plugin::pcsc::cpp::CardTerminals;
using keyple::plugin::pcsc::cpp::TerminalFactory;
using keyple::plugin::pcsc::cpp::exception::CardException;
using keyple::plugin::pcsc::cpp::exception::CardNotPresentException;

PcscReaderAdapter::PcscReaderAdapter(
    std::shared_ptr<CardTerminal> terminal,
    std::shared_ptr<PcscPluginAdapter> pluginAdapter,
    const int cardMonitoringCycleDuration)
: mIsInitialized(false)
, mIsPhysicalChannelOpen(false)
, mCommunicationTerminal(terminal)
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

    /*
     * Create a separate PC/SC context for monitoring operations to avoid
     * contention under Linux. This is critical because Linux pcsc-lite does
     * not handle concurrent access to a single SCARDCONTEXT as robustly as
     * Windows (see threading differences documentation)
     */
    mMonitoringTerminal = createMonitoringTerminal(terminal->getName());
}

std::shared_ptr<CardTerminal>
PcscReaderAdapter::createMonitoringTerminal(const std::string& terminalName)
{
    try {
        /* Attempt to create a new TerminalFactory instance to get a separate
         * PC/SC context */
        std::shared_ptr<TerminalFactory> monitoringFactory
            = TerminalFactory::getDefault();
        std::shared_ptr<CardTerminals> monitoringTerminals
            = monitoringFactory->terminals();

        /* Find the terminal with the same name in the new context */
        for (const auto& t : monitoringTerminals->list()) {
            if (t->getName() == terminalName) {
                mLogger->debug(
                    "[readerExt=%] Separate monitoring context created to "
                    "improve Linux compatibility\n",
                    terminalName);
                return t;
            }
        }

        /* Terminal not found in new context, fall back to same terminal */
        mLogger->warn(
            "[readerExt=%] Could not find terminal in separate context, "
            "using shared context (may cause issues on Linux)\n",
            terminalName);
        return mCommunicationTerminal;

    } catch (const Exception& e) {
        /* Failed to create separate context, fall back to same terminal */
        mLogger->warn(
            "[readerExt=%] Could not create separate monitoring context "
            "[reason=%], using shared context (may cause issues on Linux)\n",
            terminalName,
            e.getMessage());
        return mCommunicationTerminal;
    }
}

void
PcscReaderAdapter::waitForCardInsertion()
{
    mLogger->trace(
        "[readerExt=%] Starting waiting card insertion [loopLatencyMs=%]\n",
        getName(),
        mCardMonitoringCycleDuration);

    /* Activate loop */
    mLoopWaitCard = true;

    try {
        while (mLoopWaitCard) {
            if (mMonitoringTerminal->waitForCardPresent(
                    mCardMonitoringCycleDuration)) {
                /* Card inserted */
                mLogger->trace("[readerExt=%] Card inserted\n", getName());
                return;
            }

            // if (Thread.interrupted()) {
            //     break;
            // }
        }

        mLogger->trace(
            "[readerExt=%] Waiting card insertion stopped\n", getName());

    } catch (const CardException& e) {
        /* Here, it is a communication failure with the reader */
        throw ReaderIOException(
            "Failed to wait for a card insertion. Reader: " + mName, e);
    }

    throw TaskCanceledException(
        "The wait for a card insertion task has been cancelled. Reader: "
        + mName);
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
        const std::string rule
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
        "[readerExt=%] Activating protocol takes no action [protocol=%]\n",
        getName(),
        readerProtocol);
}

void
PcscReaderAdapter::deactivateProtocol(const std::string& readerProtocol)
{
    mLogger->trace(
        "[readerExt=%] de-activating protocol takes no action [protocol=%]\n",
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
            "[readerExt=%] Opening card physical channel [protocol=%]\n",
            getName(),
            mProtocol);

        mCard = mCommunicationTerminal->connect(mProtocol);
        if (mIsModeExclusive) {
            mCard->beginExclusive();
            mLogger->debug(
                "[readerExt=%] Card physical channel opened [mode=EXCLUSIVE]"
                "\n",
                getName());

        } else {
            mLogger->debug(
                "[readerExt=%] Card physical channel opened [mode=SHARED]\n",
                getName());
        }

        mChannel = mCard->getBasicChannel();

    } catch (const CardNotPresentException& e) {
        throw CardIOException("Card removed. Reader: " + mName, e);

    } catch (const CardException& e) {
        throw ReaderIOException(
            "Failed to open the physical channel. Reader: " + mName, e);
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
            const DisconnectionMode effectiveMode
                = isCurrentProtocol(
                      PcscCardCommunicationProtocol::INNOVATRON_B_PRIME
                          .getName())
                    ? DisconnectionMode::UNPOWER
                    : mDisconnectionMode;

            mCard->disconnect(
                effectiveMode == DisconnectionMode::UNPOWER
                || effectiveMode == DisconnectionMode::RESET);
            /* Reset the reader state to avoid bad card detection next time. */
            resetReaderState(effectiveMode);
        }

    } catch (const CardNotPresentException& e) {
        resetContext();
        throw CardIOException("Card removed. Reader: " + mName, e);

    } catch (const CardException& e) {
        resetContext();
        throw ReaderIOException(
            "Failed to close the physical channel. Reader: " + mName, e);
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

void PcscReaderAdapter::resetReaderState(const DisconnectionMode effectiveMode)
{
    try {
        if (effectiveMode == DisconnectionMode::UNPOWER) {
            mCommunicationTerminal->connect("*")->disconnect(false);
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
        const bool isCardPresent = mCommunicationTerminal->isCardPresent();
        closePhysicalChannelSafely();

        return isCardPresent;

    } catch (const CardException& e) {
        throw ReaderIOException(
            "Failed to check card presence. Reader: " + mName, e);
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
                "Card is not present. Unable to transmit APDU. Reader: "
                + mName,
                e);

        } catch (const CardException& e) {
            if (e.getMessage().find("CARD") != std::string::npos ||
                e.getMessage().find("NOT_TRANSACTED") != std::string::npos ||
                e.getMessage().find("INVALID_ATR") != std::string::npos) {
                throw CardIOException(
                    "Failed to communicate with card. Unable to transmit "
                    "APDU. Reader: "
                    + mName,
                    e);

            } else {
                throw ReaderIOException(
                    "Failed to communicate with card reader. Unable to "
                    "transmit APDU. Reader: "
                    + mName,
                    e);
            }

        } catch (const IllegalStateException& e) {
            /* Card could have been removed prematurely */
            throw CardIOException(
                "Card could have been removed prematurely. Unable to "
                "transmit APDU. Reader: "
                + mName,
                e);

        } catch (const IllegalArgumentException& e) {
            /* Card could have been removed prematurely */
            throw CardIOException(
                "Card could have been removed prematurely. Unable to "
                "transmit APDU. Reader: "
                + mName,
                e);
        }

    } else {
        /* Could occur if the card was removed */
        throw CardIOException(
            "Card channel is null. Unable to transmit APDU. Reader: "
            + mName);
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
    doWaitForCardRemoval(
        !isCurrentProtocol(
            PcscCardCommunicationProtocol::INNOVATRON_B_PRIME.getName()));
}

void
PcscReaderAdapter::stopCardPresenceMonitoringDuringProcessing()
{
    stopWaitForCardRemoval();
}

void
PcscReaderAdapter::waitForCardRemoval()
{
    doWaitForCardRemoval(true);
}

void
PcscReaderAdapter::doWaitForCardRemoval(const bool allowPolling)
{
    mLogger->trace("[readerExt=%] Starting waiting card removal\n", mName);

    mLoopWaitCardRemoval = true;

    try {
        if (allowPolling && mDisconnectionMode == DisconnectionMode::UNPOWER) {
            waitForCardRemovalByPolling();
        } else {
            waitForCardRemovalStandard();
        }

    } catch (const ReaderIOException&) {
        if (mLoopWaitCardRemoval) {
            try {
                disconnect();

            } catch (const Exception& e) {
                mLogger->warn(
                    "[readerExt=%] Failed to disconnect card during card "
                    "removal sequence [reason=%]\n",
                    mName,
                    e.getMessage());
            }
        }
        throw;
    }

    /* Finally */
    if (mLoopWaitCardRemoval) {
        try {
            disconnect();

        } catch (const Exception& e) {
            mLogger->warn(
                "[readerExt=%] Failed to disconnect card during card removal "
                "sequence [reason=%]\n",
                mName,
                e.getMessage());
        }
    }

    if (!mLoopWaitCardRemoval) {
        mLogger->trace(
            "[readerExt=%] Waiting card removal stopped\n", mName);
    } else {
        mLogger->trace("[readerExt=%] Card removed\n", mName);
    }

    if (!mLoopWaitCardRemoval) {
        throw TaskCanceledException(
            "The wait for the card removal task has been cancelled. "
            "Reader: "
            + mName);
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
            "[readerExt=%] Expected IOException received while waiting for "
            "card removal [reason=%]\n",
            getName(),
            e.getMessage());

    } catch (const ReaderIOException& e) {
        mLogger->trace(
            "[readerExt=%] Expected IOException received while waiting for "
            "card removal [reason=%]\n",
            getName(),
            e.getMessage());

    } catch (const InterruptedException& e) {
        mLogger->trace(
            "[readerExt=%] InterruptedException received while waiting for "
            "card removal: %\n",
            getName(),
            e.getMessage());
        // Thread::currentThread().interrupt();
    }
  }

void PcscReaderAdapter::waitForCardRemovalStandard()
{
    try {
        while (mLoopWaitCardRemoval) {
            if (mMonitoringTerminal->waitForCardAbsent(
                    mCardMonitoringCycleDuration)) {
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
            "Failed to wait for the card removal. Reader: " + mName, e);
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
    mLogger->info(
        "[readerExt=%] Set sharing mode [value=%]\n", getName(), sharingMode);

    if (sharingMode == SharingMode::SHARED) {
        /* If a card is present, change the mode immediately */
        if (mCard != nullptr) {
            try {
                mCard->endExclusive();

            } catch (const CardException& e) {
                throw IllegalStateException(
                    "Failed to disable exclusive mode. Reader: " + mName, e);
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
    mLogger->info(
        "[readerExt=%] Set contactless type [value=%]\n",
        getName(),
        contactless);

    mIsContactless = contactless;
    mIsInitialized = true;

    return *this;
}

PcscReader&
PcscReaderAdapter::setIsoProtocol(const IsoProtocol& isoProtocol)
{
    mLogger->info(
        "[readerExt=%] Set ISO protocol [protocol=%, value=%]\n",
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
    mLogger->info(
        "[readerExt=%] Set disconnection mode [value=%]\n",
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
            std::shared_ptr<Card> virtualCard
                = mCommunicationTerminal->connect("DIRECT");
            response = virtualCard->transmitControlCommand(controlCode, command);
            virtualCard->disconnect(false);
        }

    } catch (const CardException& e) {
        throw IllegalStateException(
            "Failed to transmit control command. Reader: " + mName, e);
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
