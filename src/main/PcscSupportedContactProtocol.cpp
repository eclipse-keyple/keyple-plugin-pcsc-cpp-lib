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

#include "PcscSupportedContactProtocol.h"

namespace keyple {
namespace plugin {
namespace pcsc {

const PcscSupportedContactProtocol PcscSupportedContactProtocol::ISO_7816_3(
    "ISO_7816_3", "3.*");
const PcscSupportedContactProtocol PcscSupportedContactProtocol::ISO_7816_3_T0(
    "ISO_7816_3_T0", "3.*");
const PcscSupportedContactProtocol PcscSupportedContactProtocol::ISO_7816_3_T1(
    "ISO_7816_3_T1", "3.*");

PcscSupportedContactProtocol::PcscSupportedContactProtocol(const std::string& name, 
                                                           const std::string& defaultRule)
: mName(name), mDefaultRule(defaultRule) {}

const std::string& PcscSupportedContactProtocol::getDefaultRule() const
{
    return mDefaultRule;
}

const std::string& PcscSupportedContactProtocol::getName() const
{
    return mName;
}

}
}
}
