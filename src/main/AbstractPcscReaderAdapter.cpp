/**************************************************************************************************
 * Copyright (c) 2021 Calypso Networks Association https://calypsonet.org/                        *
 *                                                                                                *
 * See the NOTICE file(s) distributed with this work for additional information regarding         *
 * copyright ownership.                                                                           *
 *                                                                                                *
 * This program and the accompanying materials are made available under the terms of the Eclipse  *
 * Public License 2.0 which is available at http://www.eclipse.org/legal/epl-2.0                  *
 *                                                                                                *
 * SPDX-License-Identifier: EPL-2.0                                                               *
 **************************************************************************************************/

#include "AbstractPcscReaderAdapter.h"

/* Keyple Core Util */
#include "ByteArrayUtil.h"
#include "IllegalArgumentException.h"
#include "IllegalStateException.h"
#include "KeypleAssert.h"

/* Keyple Core Plugin */
#include "CardIOException.h"
#include "ReaderIOException.h"
#include "TaskCanceledException.h"

/* Keyple Plugin Pcsc */
#include "CardException.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::plugin;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::plugin::pcsc::cpp::exception;

const long AbstractPcscReaderAdapter::REMOVAL_LATENCY = 500;

AbstractPcscReaderAdapter::AbstractPcscReaderAdapter(
  std::shared_ptr<CardTerminal> terminal, std::shared_ptr<AbstractPcscPluginAdapter> pluginAdapter)
: mTerminal(terminal),
  mName(terminal ? terminal->getName() : ""),
  mPluginAdapter(pluginAdapter),
  mIsContactless(false),
  mIsInitialized(false),
  mIsPhysicalChannelOpen(false),
  mProtocol(IsoProtocol::ANY.getValue()),
  mIsModeExclusive(true),
  mDisconnectionMode(DisconnectionMode::RESET),
  mLoopWaitCardRemoval(false)
{
    /* C++ addon */
    if (!terminal) {
        throw IllegalArgumentException("Terminal should not be null");
    }
}

std::shared_ptr<CardTerminal> AbstractPcscReaderAdapter::getTerminal() const
{
    return mTerminal;
}

const std::string& AbstractPcscReaderAdapter::getName() const
{
    return mName;
}

bool AbstractPcscReaderAdapter::isProtocolSupported(const std::string& readerProtocol) const
{
    /* C++ specific: getProtocolRule throws when protocol not found instead of returning null */
    try {
        const std::string rulmIsPhysicalChannelOpen= mPluginAdapter->getProtocolRule(readerProtocol);
        return true;

    } catch (const IllegalArgumentException& e) {
        (void)e;
    }

    return false;
}

void AbstractPcscReaderAdapter::activateProtocol(const std::string& readerProtocol)
{
    mLogger->trace("%: activating the % protocol causes no action to be taken\n",
                   getName(),
                   readerProtocol);
}

void AbstractPcscReaderAdapter::deactivateProtocol(const std::string& readerProtocol)
{
    mLogger->trace("%: deactivating the % protocol causes no action to be taken\n",
                   getName(),
                   readerProtocol);
}

bool AbstractPcscReaderAdapter::isCurrentProtocol(const std::string& readerProtocol) const
{
    bool isCurrentProtocol = false;

    /* C++ specific: getProtocolRule throws when protocol not found instead of returning null */
    try {
        const std::string protocolRule = mPluginAdapter->getProtocolRule(readerProtocol);
        if (!protocolRule.empty()) {
            const std::string atr = ByteArrayUtil::toHex(mTerminal->getATR());
            isCurrentProtocol = Pattern::compile(protocolRule)->matcher(atr)->matches();
        }

        return isCurrentProtocol;

    } catch (const IllegalArgumentException& e) {
        (void)e;
    }

    return false;
}

void AbstractPcscReaderAdapter::openPhysicalChannel()
{
    /*
     * Init of the card physical channel: if not yet established, opening of a new physical channel
     */
    try {
        if (!mIsPhysicalChannelOpen) {
            mLogger->debug("%: opening of a card physical channel for protocol '%'\n",
                           getName(),
                           mProtocol);
            mTerminal->openAndConnect(mProtocol);
            if (mIsModeExclusive) {
                mTerminal->beginExclusive();
                mLogger->debug("%: opening of a card physical channel in exclusive mode\n",
                               getName());
            } else {
                mLogger->debug("%: opening of a card physical channel in shared mode\n", getName());
            }
        }

        mIsPhysicalChannelOpen = true;

    } catch (const CardException& e) {
        throw ReaderIOException(getName() + ": Error while opening Physical Channel",
                                std::make_shared<CardException>(e));
    }
}

void AbstractPcscReaderAdapter::closePhysicalChannel()
{
    try {
        if (mIsPhysicalChannelOpen) {
            mTerminal->closeAndDisconnect(mDisconnectionMode);
        } else {
            mLogger->debug("%: card object found null when closing the physical channel\n",
                           getName());
        }
    } catch (const CardException& e) {
        throw ReaderIOException("Error while closing physical channel",
                                std::make_shared<CardException>(e));
    }

    mIsPhysicalChannelOpen = false;
}

bool AbstractPcscReaderAdapter::isPhysicalChannelOpen() const
{
    return mIsPhysicalChannelOpen;
}

bool AbstractPcscReaderAdapter::checkCardPresence()
{
    try {
        return mTerminal->isCardPresent(false);
    } catch (const CardException& e) {
        throw ReaderIOException("Exception occurred in isCardPresent",
                                std::make_shared<CardException>(e));
    }
}

const std::string AbstractPcscReaderAdapter::getPowerOnData() const
{
    return ByteArrayUtil::toHex(mTerminal->getATR());
}

const std::vector<uint8_t> AbstractPcscReaderAdapter::transmitApdu(
    const std::vector<uint8_t>& apduCommandData)
{
    std::vector<uint8_t> apduResponseData;

    if (mIsPhysicalChannelOpen) {
        try {
            apduResponseData = mTerminal->transmitApdu(apduCommandData);
        } catch (const CardException& e) {
            if (e.getMessage().find("REMOVED") != std::string::npos) {
                throw CardIOException(getName() + ":" + e.getMessage(),
                                      std::make_shared<CardException>(e));
            } else {
                throw ReaderIOException(getName() + ":" + e.getMessage(),
                                        std::make_shared<CardException>(e));
            }
        } catch (const IllegalStateException& e) {
            /* Card could have been removed prematurely */
            throw CardIOException(getName() + ":" + e.getMessage(),
                                  std::make_shared<IllegalStateException>(e));
        }
    } else {
        /* Could occur if the card was removed */
        throw CardIOException(getName() + ": null channel.");
    }

    return apduResponseData;
}

bool AbstractPcscReaderAdapter::isContactless()
{
    if (!mIsInitialized) {
        /*
         * First time initialisation, the transmission mode has not yet been determined or fixed
         * explicitly, let's ask the plugin to determine it (only once)
         */
        mIsContactless = mPluginAdapter->isContactless(getName());
    }

    return mIsContactless;
}

void AbstractPcscReaderAdapter::onUnregister()
{
    /* Nothing to do here in this reader */
}

void AbstractPcscReaderAdapter::onStartDetection()
{
    /* Nothing to do here in this reader */
}

void AbstractPcscReaderAdapter::onStopDetection()
{
    /* Nothing to do here in this reader */
}

PcscReader& AbstractPcscReaderAdapter::setSharingMode(const SharingMode sharingMode)
{
    mLogger->trace("%: set sharing mode to %\n", getName(), sharingMode);

    if (sharingMode == SharingMode::SHARED) {
        /* If a card is present, change the mode immediately */
        if (mIsPhysicalChannelOpen) {
            try {
                mTerminal->endExclusive();
            } catch (const CardException& e) {
                throw IllegalStateException("Couldn't disable exclusive mode",
                                            std::make_shared<CardException>(e));
            }
        }

        mIsModeExclusive = false;
    } else if (sharingMode == SharingMode::EXCLUSIVE) {
        mIsModeExclusive = true;
    }

    return *this;
}

PcscReader& AbstractPcscReaderAdapter::setContactless(const bool contactless)
{
    mLogger->trace("%: set contactless type: %\n", getName(), contactless);

    mIsContactless = contactless;
    mIsInitialized = true;

    return *this;
}

PcscReader& AbstractPcscReaderAdapter::setIsoProtocol(const IsoProtocol& isoProtocol)
{
    mLogger->trace("%: set ISO protocol to % (%)\n", getName(), isoProtocol, isoProtocol.getValue());

    mProtocol = isoProtocol.getValue();

    return *this;
}

PcscReader& AbstractPcscReaderAdapter::setDisconnectionMode(
    const DisconnectionMode disconnectionMode)
{
    mLogger->trace("%: set disconnection to %\n", getName(), disconnectionMode);

    mDisconnectionMode = disconnectionMode;

    return *this;
}

void AbstractPcscReaderAdapter::waitForCardRemoval()
{
    mLogger->trace("%: start waiting for the removal of the card in a loop with a latency of % " \
                   "ms\n",
                   getName(),
                   REMOVAL_LATENCY);


    /* Activate loop */
    mLoopWaitCardRemoval = true;

    try {
        while (mLoopWaitCardRemoval) {
            if (getTerminal()->waitForCardAbsent(REMOVAL_LATENCY)) {
                /* Card removed */
                mLogger->trace("%: card removed\n", getName());
                return;
            }

            // if (Thread.interrupted()) {
            //     break;
            // }
        }
    } catch (const CardException& e) {
        /* Here, it is a communication failure with the reader */
        throw ReaderIOException(getName() +
                                ": an error occurred while waiting for the card removal.",
                                std::make_shared<CardException>(e));
    }

    throw TaskCanceledException(getName() +
                                ": the wait for the card removal task has been cancelled.");
}

void AbstractPcscReaderAdapter::stopWaitForCardRemoval()
{
    mLogger->trace("%: stop waiting for the card removal requested\n", getName());

    mLoopWaitCardRemoval = false;
}

}
}
}
