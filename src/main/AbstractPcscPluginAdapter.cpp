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

#include "AbstractPcscPluginAdapter.h"

/* Keyple Plugin Pcsc */
#include "PcscSupportedContactlessProtocol.h"
#include "PcscSupportedContactProtocol.h"

/* Keyple Core Util */
#include "Pattern.h"
#include "KeypleStd.h"
#include "IllegalArgumentException.h"
#include "IllegalStateException.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;

const int AbstractPcscPluginAdapter::MONITORING_CYCLE_DURATION_MS = 1000;

AbstractPcscPluginAdapter::AbstractPcscPluginAdapter(const std::string& name)
: mName(name),
  mContactReaderIdentificationFilter(""), 
  mContactlessReaderIdentificationFilter("")
{
    mProtocolRulesMap = {
        /* Contactless protocols */
        {
            PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
            PcscSupportedContactlessProtocol::ISO_14443_4.getDefaultRule()
        }, {
            PcscSupportedContactlessProtocol::INNOVATRON_B_PRIME_CARD.getName(),
            PcscSupportedContactlessProtocol::INNOVATRON_B_PRIME_CARD.getDefaultRule()
        }, {
            PcscSupportedContactlessProtocol::MIFARE_ULTRA_LIGHT.getName(),
            PcscSupportedContactlessProtocol::MIFARE_ULTRA_LIGHT.getDefaultRule()
        }, {
            PcscSupportedContactlessProtocol::MIFARE_CLASSIC.getName(),
            PcscSupportedContactlessProtocol::MIFARE_CLASSIC.getDefaultRule()
        }, {
            PcscSupportedContactlessProtocol::MIFARE_DESFIRE.getName(),
            PcscSupportedContactlessProtocol::MIFARE_DESFIRE.getDefaultRule()
        }, {
            PcscSupportedContactlessProtocol::MEMORY_ST25.getName(),
            PcscSupportedContactlessProtocol::MEMORY_ST25.getDefaultRule()
        }, 
        /* Contact protocols */
        {
            PcscSupportedContactProtocol::ISO_7816_3.getName(),
            PcscSupportedContactProtocol::ISO_7816_3.getDefaultRule()
        }, {
            PcscSupportedContactProtocol::ISO_7816_3_T0.getName(),
            PcscSupportedContactProtocol::ISO_7816_3_T0.getDefaultRule()
        }, {
            PcscSupportedContactProtocol::ISO_7816_3_T1.getName(),
            PcscSupportedContactProtocol::ISO_7816_3_T1.getDefaultRule()
        }
    };
}

AbstractPcscPluginAdapter& AbstractPcscPluginAdapter::setContactReaderIdentificationFilter(
    const std::string& contactReaderIdentificationFilter) 
{
    if (mContactReaderIdentificationFilter != "") {
        mLogger->trace("%: contact reader identification filter set to %\n",
                       getName(),
                       contactReaderIdentificationFilter);
    } else {
        mLogger->trace("%: no contact reader identification filter set\n", getName());
    }
    
    mContactReaderIdentificationFilter = contactReaderIdentificationFilter;

    return *this;
}

AbstractPcscPluginAdapter& AbstractPcscPluginAdapter::setContactlessReaderIdentificationFilter(
    const std::string& contactlessReaderIdentificationFilter)
{
    
    if (contactlessReaderIdentificationFilter != "") {
        mLogger->trace("%: contactless reader identification filter set to %\n",
                       getName(),
                       mContactlessReaderIdentificationFilter);
    } else {
        mLogger->trace("%: no contactless reader identification filter set\n", getName());
    }
    
    mContactlessReaderIdentificationFilter = contactlessReaderIdentificationFilter;
    
    return *this;
}

AbstractPcscPluginAdapter& AbstractPcscPluginAdapter::addProtocolRulesMap(
    const std::map<std::string, std::string> protocolRulesMap) 
{
    if (!protocolRulesMap.empty()) {
        mLogger->trace("%: protocol identification rules updated with %\n",
                       getName(),
                       protocolRulesMap);
    } else {
        mLogger->trace("%: using default protocol identification rules\n", getName());
    }

    mProtocolRulesMap.insert(protocolRulesMap.begin(), protocolRulesMap.end());
    
    return *this;
}

const std::string& AbstractPcscPluginAdapter::getProtocolRule(const std::string& readerProtocol) 
    const
{
    const auto it = mProtocolRulesMap.find(readerProtocol);
    if (it != mProtocolRulesMap.end()) {
        return it->second;
    } else {
        /*
         * C++ specific, cannot return null here, to check if not a problem with the flow
         * somewhere...
         */
        throw IllegalArgumentException("bad protocol");
    }
}

bool AbstractPcscPluginAdapter::isContactless(const std::string& readerName)
{
    std::unique_ptr<Pattern> p = Pattern::compile(mContactReaderIdentificationFilter);
    if (p->matcher(readerName)->matches()) {
        return false;
    }

    p = Pattern::compile(mContactlessReaderIdentificationFilter);
    if (p->matcher(readerName)->matches()) {
        return true;
    }

    throw IllegalStateException("Unable to determine the transmission mode for reader " +
                                readerName);
}

const std::vector<std::shared_ptr<CardTerminal>> AbstractPcscPluginAdapter::getCardTerminalList() 
    const
{
    /* Parse the current readers list to create the ReaderSpi(s) associated with new reader(s) */
    const std::vector<std::shared_ptr<CardTerminal>> terminals = getCardTerminals();
    if (!terminals.size())
        mLogger->error("No reader available\n");
    
    return terminals;
}

const std::string& AbstractPcscPluginAdapter::getName() const
{
    return mName;
}

const std::vector<std::shared_ptr<ReaderSpi>> AbstractPcscPluginAdapter::searchAvailableReaders()
{
    std::vector<std::shared_ptr<ReaderSpi>> readerSpis;

    for (const auto& terminal : getCardTerminalList()) {
        readerSpis.push_back(createReader(terminal));
    }

    mLogger->trace("%: available readers %\n", getName(), readerSpis);

    return readerSpis;
}

void AbstractPcscPluginAdapter::onUnregister()
{
    /* Nothing to do here in this plugin */
}

int AbstractPcscPluginAdapter::getMonitoringCycleDuration() const
{
    return MONITORING_CYCLE_DURATION_MS;
}

const std::vector<std::string> AbstractPcscPluginAdapter::searchAvailableReaderNames()
{
    std::vector<std::string> readerNames;

    for (const auto& terminal : getCardTerminalList()) {
        readerNames.push_back(terminal->getName());
    }

    mLogger->trace("%: available readers names %\n", getName(), readerNames);
    
    return readerNames;
}

std::shared_ptr<ReaderSpi> AbstractPcscPluginAdapter::searchReader(const std::string& readerName) 
{
    mLogger->trace("%: search reader: %\n", getName(), readerName);
    
    for (const auto& terminal : getCardTerminalList()) {
        if (readerName == terminal->getName()) {
            mLogger->trace("%: reader: % found\n", getName(), readerName);
            return createReader(terminal);
        }
    }
    
    mLogger->trace("%: reader: % not found\n", getName(), readerName);
    
    return nullptr;
}

}
}
}
