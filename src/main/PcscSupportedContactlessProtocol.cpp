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

#include "PcscSupportedContactlessProtocol.h"

namespace keyple {
namespace plugin {
namespace pcsc {

const PcscSupportedContactlessProtocol PcscSupportedContactlessProtocol::ISO_14443_4(
    "ISO_14443_4", "3B8880....................|3B8B80.*|3B8C800150.*|.*4F4D4141544C4153.*");
const PcscSupportedContactlessProtocol PcscSupportedContactlessProtocol::INNOVATRON_B_PRIME_CARD(
    "INNOVATRON_B_PRIME_CARD", "3B8F8001805A0...................829000..");
const PcscSupportedContactlessProtocol PcscSupportedContactlessProtocol::MIFARE_ULTRA_LIGHT(
    "MIFARE_ULTRA_LIGHT", "3B8F8001804F0CA0000003060300030000000068");
const PcscSupportedContactlessProtocol PcscSupportedContactlessProtocol::MIFARE_CLASSIC(
    "MIFARE_CLASSIC", "3B8F8001804F0CA000000306030001000000006A");
const PcscSupportedContactlessProtocol PcscSupportedContactlessProtocol::MIFARE_DESFIRE(
    "MIFARE_DESFIRE", "3B8180018080");
const PcscSupportedContactlessProtocol PcscSupportedContactlessProtocol::MEMORY_ST25(
    "MEMORY_ST25", "3B8F8001804F0CA000000306070007D0020C00B6");

PcscSupportedContactlessProtocol::PcscSupportedContactlessProtocol(const std::string& name,
                                                                   const std::string& defaultRule)
: mName(name), mDefaultRule(defaultRule) {}

const std::string& PcscSupportedContactlessProtocol::getDefaultRule() const
{
    return mDefaultRule;
}

const std::string& PcscSupportedContactlessProtocol::getName() const
{
    return mName;
}

}
}
}
