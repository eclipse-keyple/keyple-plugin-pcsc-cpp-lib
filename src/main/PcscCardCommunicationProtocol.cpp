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

#include "keyple/plugin/pcsc/PcscCardCommunicationProtocol.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

const PcscCardCommunicationProtocol
    PcscCardCommunicationProtocol::ISO_14443_4(
        "ISO_14443_4", "3B8[0-9A-F]8001(?!.*5A0A)(?!804F0CA000000306).*");

const PcscCardCommunicationProtocol
    PcscCardCommunicationProtocol::INNOVATRON_B_PRIME(
        "INNOVATRON_B_PRIME", "3B8.8001(80)?5A0A.*");

const PcscCardCommunicationProtocol
    PcscCardCommunicationProtocol::MIFARE_ULTRALIGHT(
        "MIFARE_ULTRALIGHT", "3B8F8001804F0CA0000003060300030.*");

const PcscCardCommunicationProtocol
    PcscCardCommunicationProtocol::ST25_SRT512(
        "ST25_SRT512", "3B8F8001804F0CA0000003060(5|6|7)0007.*");

const PcscCardCommunicationProtocol
    PcscCardCommunicationProtocol::ISO_7816_3("ISO_7816_3", "3.*");

PcscCardCommunicationProtocol::PcscCardCommunicationProtocol(
    const std::string& name, const std::string& defaultRule)
: mName(name)
, mDefaultRule(defaultRule)
{
}

const std::string&
PcscCardCommunicationProtocol::getDefaultRule() const
{
    return mDefaultRule;
}

const std::string&
PcscCardCommunicationProtocol::getName() const
{
    return mName;
}

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */