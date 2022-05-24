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

#include "PcscReader.h"

namespace keyple {
namespace plugin {
namespace pcsc {

/* ISO PROTOCOL --------------------------------------------------------------------------------- */

const PcscReader::IsoProtocol PcscReader::IsoProtocol::ANY("*");
const PcscReader::IsoProtocol PcscReader::IsoProtocol::T0("T=0");
const PcscReader::IsoProtocol PcscReader::IsoProtocol::T1("T=1");
const PcscReader::IsoProtocol PcscReader::IsoProtocol::TCL("T=CL");

PcscReader::IsoProtocol::IsoProtocol(const std::string& value) : mValue(value) {}

const std::string& PcscReader::IsoProtocol::getValue() const
{
    return mValue;
}

std::ostream& operator<<(std::ostream& os, const PcscReader::IsoProtocol& ip)
{
    os << "ISO_PROTOCOL = ";
    
    if (ip == PcscReader::IsoProtocol::ANY) {
        os << "ANY";
    } else if (ip == PcscReader::IsoProtocol::T0) {
        os << "T0";
    } else if (ip == PcscReader::IsoProtocol::T1) {
        os << "T1";
    } else if (ip == PcscReader::IsoProtocol::TCL) {
        os << "TCL";
    } else {
        os << "UNKNOWN";
    }

    return os;
}

bool PcscReader::IsoProtocol::operator==(const PcscReader::IsoProtocol& ip) const
{
    return ip.getValue() == this->getValue();
}

/* SHARING MODE --------------------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const PcscReader::SharingMode sm)
{
    os << "SHARING_MODE = ";
    
    switch(sm) {
    case PcscReader::SharingMode::EXCLUSIVE:
        os << "EXCLUSIVE";
        break;
    case PcscReader::SharingMode::SHARED:
        os << "SHARED";
        break;
    default:
        os << "UNKNOWN";
        break;
    }

    return os;
}

/* DISCONNECTION MODE --------------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const PcscReader::DisconnectionMode dm)
{
    os << "DISCONNECTION_MODE = ";
    
    switch(dm) {
    case PcscReader::DisconnectionMode::RESET:
        os << "RESET";
        break;
    case PcscReader::DisconnectionMode::LEAVE:
        os << "LEAVE";
        break;
    default:
        os << "UNKNOWN";
        break;
    }

    return os;
}


}
}
}
