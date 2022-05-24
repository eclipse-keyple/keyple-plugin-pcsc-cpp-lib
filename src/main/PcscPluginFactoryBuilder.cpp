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

#include "PcscPluginFactoryBuilder.h"

/* Keyple Plugin Pcsc */
#include "PcscPluginFactoryAdapter.h"

/* Keyple Core Util */
#include "KeypleAssert.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::util;

using Builder = PcscPluginFactoryBuilder::Builder;

/* BUILDER -------------------------------------------------------------------------------------- */

Builder::Builder() {}

Builder& Builder::useContactReaderIdentificationFilter(
    const std::string contactReaderIdentificationFilter)
{
    Assert::getInstance().notEmpty(contactReaderIdentificationFilter,
                                   "contactReaderIdentificationFilter");

    mContactReaderIdentificationFilter = contactReaderIdentificationFilter;

    return *this;
}

Builder& Builder::useContactlessReaderIdentificationFilter(
    const std::string contactlessReaderIdentificationFilter)
{
    Assert::getInstance().notEmpty(contactlessReaderIdentificationFilter,
                                   "contactlessReaderIdentificationFilter");

    mContactlessReaderIdentificationFilter = contactlessReaderIdentificationFilter;

    return *this;
}

Builder& Builder::updateProtocolIdentificationRule(const std::string& readerProtocolName,
                                                   const std::string& protocolRule)
{
    Assert::getInstance().notEmpty(readerProtocolName, "readerProtocolName");
    if (protocolRule == "") {
        /* Disable the protocol by defining a regex that always fails */
        mProtocolRulesMap.insert({readerProtocolName, "X"});
    } else {
        mProtocolRulesMap.insert({readerProtocolName, protocolRule});
    }

    return *this;
}

std::shared_ptr<PcscPluginFactory> PcscPluginFactoryBuilder::Builder::build()
{
    return std::make_shared<PcscPluginFactoryAdapter>(mContactReaderIdentificationFilter,
                                                      mContactlessReaderIdentificationFilter,
                                                      mProtocolRulesMap);
}

/* PCSC PLUGIN FACTORY BUILDER ------------------------------------------------------------------ */

std::unique_ptr<Builder> PcscPluginFactoryBuilder::builder()
{
    return std::unique_ptr<Builder>(new Builder());
}

}
}
}
