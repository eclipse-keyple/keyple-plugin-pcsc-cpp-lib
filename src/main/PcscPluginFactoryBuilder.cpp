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

#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"

#include "keyple/core/util/KeypleAssert.hpp"
#include "keyple/core/util/cpp/Pattern.hpp"
#include "keyple/core/util/cpp/exception/Exception.hpp"
#include "keyple/core/util/cpp/exception/IllegalArgumentException.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryAdapter.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::util::Assert;
using keyple::core::util::cpp::Pattern;
using keyple::core::util::cpp::exception::Exception;
using keyple::core::util::cpp::exception::IllegalArgumentException;

using Builder = PcscPluginFactoryBuilder::Builder;

const std::string Builder::DEFAULT_CONTACTLESS_READER_FILTER
    = ".*(contactless|ask logo|acs acr122).*";

/* BUILDER ------------------------------------------------------------------ */

Builder::Builder()
: mContactlessReaderIdentificationFilterPattern(
    Pattern::compile(Builder::DEFAULT_CONTACTLESS_READER_FILTER))
, mCardMonitoringCycleDuration(500)
{
}

Builder&
Builder::useContactReaderIdentificationFilter(
    const std::string& /*contactReaderIdentificationFilter*/)
{
    return *this;
}

Builder&
Builder::useContactlessReaderIdentificationFilter(
    const std::string& contactlessReaderIdentificationFilter)
{
    Assert::getInstance().notEmpty(
        contactlessReaderIdentificationFilter,
        "contactlessReaderIdentificationFilter");

    try {
        mContactlessReaderIdentificationFilterPattern
            = Pattern::compile(contactlessReaderIdentificationFilter);

    } catch (const Exception& e) {
        throw IllegalArgumentException(
            "Bad regular expression.", e);
    }

    return *this;
}

Builder&
Builder::updateProtocolIdentificationRule(
    const std::string& readerProtocolName, const std::string& protocolRule)
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

Builder&
Builder::setCardMonitoringCycleDuration(const int cycleDuration)
{
    Assert::getInstance().greaterOrEqual(cycleDuration, 1, "cycleDuration");

    mCardMonitoringCycleDuration = cycleDuration;

    return *this;
}

std::shared_ptr<PcscPluginFactory>
PcscPluginFactoryBuilder::Builder::build()
{
    return std::make_shared<PcscPluginFactoryAdapter>(
            mContactlessReaderIdentificationFilterPattern,
            mProtocolRulesMap,
            mCardMonitoringCycleDuration);
}

/* PCSC PLUGIN FACTORY BUILDER
 * ------------------------------------------------------------------ */

std::unique_ptr<Builder>
PcscPluginFactoryBuilder::builder()
{
    return std::unique_ptr<Builder>(new Builder());
}

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
