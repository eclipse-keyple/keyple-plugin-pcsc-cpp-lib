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

#include "keyple/plugin/pcsc/PcscPluginAdapter.hpp"

#include "keyple/core/plugin/PluginIOException.hpp"
#include "keyple/core/util/cpp/KeypleStd.hpp"
#include "keyple/core/util/cpp/StringUtils.hpp"
#include "keyple/core/util/cpp/exception/Exception.hpp"
#include "keyple/core/util/cpp/exception/IllegalArgumentException.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryAdapter.hpp"
#include "keyple/plugin/pcsc/PcscReaderAdapter.hpp"
#include "keyple/plugin/pcsc/PcscCardCommunicationProtocol.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactProtocol.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactlessProtocol.hpp"
#include "keyple/plugin/pcsc/cpp/CardTerminal.hpp"
#include "keyple/plugin/pcsc/cpp/CardTerminals.hpp"
#include "keyple/plugin/pcsc/cpp/TerminalFactory.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardTerminalException.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::plugin::PluginIOException;
using keyple::core::util::cpp::StringUtils;
using keyple::core::util::cpp::exception::Exception;
using keyple::core::util::cpp::exception::IllegalArgumentException;
using keyple::plugin::pcsc::cpp::CardTerminal;
using keyple::plugin::pcsc::cpp::CardTerminals;
using keyple::plugin::pcsc::cpp::TerminalFactory;
using keyple::plugin::pcsc::cpp::exception::CardTerminalException;

std::shared_ptr<PcscPluginAdapter> PcscPluginAdapter::INSTANCE;

const int PcscPluginAdapter::MONITORING_CYCLE_DURATION_MS = 1000;

PcscPluginAdapter::PcscPluginAdapter()
: PcscPlugin()
, ObservablePluginSpi()
, mIsCardTerminalsInitialized(false)
{
    /* Initializes the protocol rules map with default values. */
    mProtocolRulesMap = {

        /* Contactless protocols */
        {PcscCardCommunicationProtocol::ISO_14443_4.getName(),
         PcscCardCommunicationProtocol::ISO_14443_4.getDefaultRule()},
        {PcscCardCommunicationProtocol::INNOVATRON_B_PRIME.getName(),
         PcscCardCommunicationProtocol::INNOVATRON_B_PRIME.getDefaultRule()},
        {PcscCardCommunicationProtocol::MIFARE_ULTRALIGHT.getName(),
         PcscCardCommunicationProtocol::MIFARE_ULTRALIGHT.getDefaultRule()},
        {PcscSupportedContactlessProtocol::MIFARE_CLASSIC.getName(),
         PcscSupportedContactlessProtocol::MIFARE_CLASSIC.getDefaultRule()},
        {PcscSupportedContactlessProtocol::MIFARE_DESFIRE.getName(),
         PcscSupportedContactlessProtocol::MIFARE_DESFIRE.getDefaultRule()},
        {PcscCardCommunicationProtocol::ST25_SRT512.getName(),
         PcscCardCommunicationProtocol::ST25_SRT512.getDefaultRule()},

        /* Contact protocols */
        {PcscCardCommunicationProtocol::ISO_7816_3.getName(),
         PcscCardCommunicationProtocol::ISO_7816_3.getDefaultRule()},

         /* Legacy protocols for compatibility */
        {PcscSupportedContactProtocol::ISO_7816_3_T0.getName(),
         PcscSupportedContactProtocol::ISO_7816_3_T0.getDefaultRule()},
        {PcscSupportedContactProtocol::ISO_7816_3_T1.getName(),
         PcscSupportedContactProtocol::ISO_7816_3_T1.getDefaultRule()}};
}

std::shared_ptr<PcscPluginAdapter>
PcscPluginAdapter::getInstance()
{
    if (!INSTANCE) {
        INSTANCE = std::make_shared<PcscPluginAdapter>();
    }

    return INSTANCE;
}

std::unique_ptr<PcscReaderAdapter>
PcscPluginAdapter::createReader(std::shared_ptr<CardTerminal> terminal)
{
    return std::unique_ptr<PcscReaderAdapter>(new PcscReaderAdapter(
        terminal, shared_from_this(), mCardMonitoringCycleDuration));
}

int
PcscPluginAdapter::getMonitoringCycleDuration() const
{
    return MONITORING_CYCLE_DURATION_MS;
}

const std::vector<std::string>
PcscPluginAdapter::searchAvailableReaderNames()
{
    std::vector<std::string> readerNames;

    mLogger->trace("Plugin [%]: search available reader\n", getName());

    for (const auto& terminal : getCardTerminalList()) {
        readerNames.push_back(terminal->getName());
    }

    mLogger->trace("Plugin [%]: readers found: %", getName(), readerNames);

    return readerNames;
}

const std::string&
PcscPluginAdapter::getName() const
{
    return PcscPluginFactoryAdapter::PLUGIN_NAME;
}

const std::vector<std::shared_ptr<ReaderSpi>>
PcscPluginAdapter::searchAvailableReaders()
{
    std::vector<std::shared_ptr<ReaderSpi>> readerSpis;

    mLogger->trace("Plugin [%]: search available readers\n", getName());

    for (const auto& terminal : getCardTerminalList()) {
        readerSpis.push_back(createReader(terminal));
    }

    for (const auto& readerSpi : readerSpis) {
        mLogger->trace(
            "Plugin [%]: reader found: %\n", getName(), readerSpi->getName());
    }

    return readerSpis;
}

void
PcscPluginAdapter::onUnregister()
{
    /* Nothing to do here in this plugin */
}

const std::vector<std::shared_ptr<CardTerminal>>
PcscPluginAdapter::getCardTerminalList()
{
    /*
     * Parse the current readers list to create the ReaderSpi(s) associated with
     * new reader(s).
     */

    try {
        if (!mIsCardTerminalsInitialized) {
            mTerminals = TerminalFactory::getDefault()->terminals();
            mIsCardTerminalsInitialized = true;
        }

        return mTerminals->list();

    } catch (const Exception& e) {
        const auto msg = e.getMessage();

        if (StringUtils::contains(msg, "SCARD_E_NO_READERS_AVAILABLE")) {
            mLogger->error("Plugin [%]: no reader available\n", getName());

        } else if (
            StringUtils::contains(msg, "SCARD_E_NO_SERVICE")
            || StringUtils::contains(msg, "SCARD_E_SERVICE_STOPPED")) {
            mLogger->error(
                "Plugin [%]: no smart card service error\n", getName());
            mIsCardTerminalsInitialized = false;

        } else if (StringUtils::contains(msg, "SCARD_F_COMM_ERROR")) {
            mLogger->error(
                "Plugin [%]: reader communication error\n", getName());

        } else {
            throw PluginIOException(
                "Could not access terminals list",
                std::make_shared<CardTerminalException>(msg));
        }
    }

    return std::vector<std::shared_ptr<CardTerminal>>(0);
}

std::shared_ptr<ReaderSpi>
PcscPluginAdapter::searchReader(const std::string& readerName)
{
    mLogger->trace("Plugin [%]: search reader [%]\n", getName(), readerName);

    const auto terminals = getCardTerminalList();
    auto it = std::find_if(
        terminals.begin(),
        terminals.end(),
        [readerName](const std::shared_ptr<CardTerminal>& terminal) {
            return terminal->getName() == readerName;
        });

    if (it != terminals.end()) {
        mLogger->trace("Plugin [%]: reader found\n", getName());
        return createReader(*it);
    }

    mLogger->trace("Plugin [%]: reader not found\n", getName());

    return nullptr;
}

const std::string&
PcscPluginAdapter::getProtocolRule(const std::string& readerProtocol) const
{
    const auto it = mProtocolRulesMap.find(readerProtocol);
    if (it != mProtocolRulesMap.end()) {
        return it->second;

    } else {
        /*
         * C++ specific, cannot return null here, to check if not a problem
         * with the flow somewhere...
         */
        throw IllegalArgumentException("bad protocol");
    }
}

bool
PcscPluginAdapter::isContactless(const std::string& readerName)
{
    return mContactlessReaderIdentificationFilterPattern->matcher(readerName)
        ->matches();
}

PcscPluginAdapter&
PcscPluginAdapter::setContactlessReaderIdentificationFilterPattern(
    const std::shared_ptr<Pattern> contactlessReaderIdentificationFilter)
{
    mContactlessReaderIdentificationFilterPattern
        = contactlessReaderIdentificationFilter;

    return *this;
}

PcscPluginAdapter&
PcscPluginAdapter::addProtocolRulesMap(
    const std::map<std::string, std::string> protocolRulesMap)
{
    if (!protocolRulesMap.empty()) {
        mLogger->info(
            "Plugin [%]: add protocol identification rules: %\n",
            getName(),
            protocolRulesMap);
    } else {
        mLogger->info(
            "Plugin [%]: use default protocol identification rules\n",
            getName());
    }

    mProtocolRulesMap.insert(protocolRulesMap.begin(), protocolRulesMap.end());

    return *this;
}

PcscPluginAdapter&
PcscPluginAdapter::setCardMonitoringCycleDuration(
    const int cardMonitoringCycleDuration)
{
    mCardMonitoringCycleDuration = cardMonitoringCycleDuration;

    return *this;
}

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
