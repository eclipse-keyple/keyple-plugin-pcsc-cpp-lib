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

#include "PcscPluginFactoryAdapter.h"

/* Keyple Plugin Pcsc */
#include "AbstractPcscPluginAdapter.h"
#include "PcscPluginAdapter.h"
#include "PcscPluginFactory.h"

/* Keyple Core Plugin */
#include "PluginApiProperties.h"

/* Keyple Core Common */
#include "CommonApiProperties.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::common;
using namespace keyple::core::plugin;

const std::string PcscPluginFactoryAdapter::PLUGIN_NAME = "PcscPlugin";

PcscPluginFactoryAdapter::PcscPluginFactoryAdapter(
  const std::string& contactReaderIdentificationFilter,
  const std::string& contactlessReaderIdentificationFilter,
  const std::map<std::string, std::string>& protocolRulesMap)
: mContactReaderIdentificationFilter(contactReaderIdentificationFilter),
  mContactlessReaderIdentificationFilter(contactlessReaderIdentificationFilter),
  mProtocolRulesMap(protocolRulesMap) {}

const std::string& PcscPluginFactoryAdapter::getPluginApiVersion() const
{
    return PluginApiProperties_VERSION;
}

const std::string& PcscPluginFactoryAdapter::getCommonApiVersion() const
{
    return CommonApiProperties_VERSION;
}

const std::string& PcscPluginFactoryAdapter::getPluginName() const
{
    return PLUGIN_NAME;
}

std::shared_ptr<PluginSpi> PcscPluginFactoryAdapter::getPlugin()
{
    std::shared_ptr<AbstractPcscPluginAdapter> plugin = PcscPluginAdapter::getInstance();
    plugin->setContactReaderIdentificationFilter(mContactReaderIdentificationFilter)
           .setContactlessReaderIdentificationFilter(mContactlessReaderIdentificationFilter)
           .addProtocolRulesMap(mProtocolRulesMap);

    return plugin;
}

}
}
}
