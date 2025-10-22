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

#include "keyple/plugin/pcsc/cpp/CardTerminal.hpp"

#include <chrono>
#include <cstdint>
#include <string>

#include "PcscUtils.hpp"

#include "keyple/core/util/cpp/KeypleStd.hpp"
#include "keyple/core/util/cpp/StringUtils.hpp"
#include "keyple/core/util/cpp/System.hpp"
#include "keyple/core/util/cpp/Thread.hpp"
#include "keyple/core/util/cpp/exception/IllegalArgumentException.hpp"
#include "keyple/core/util/cpp/exception/RuntimeException.hpp"
#include "keyple/plugin/pcsc/cpp/CardTerminals.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardException.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardNotPresentException.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardTerminalException.hpp"


namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using keyple::core::util::cpp::StringUtils;
using keyple::core::util::cpp::System;
using keyple::core::util::cpp::Thread;
using keyple::core::util::cpp::exception::IllegalArgumentException;
using keyple::core::util::cpp::exception::RuntimeException;
using keyple::plugin::pcsc::cpp::exception::CardException;
using keyple::plugin::pcsc::cpp::exception::CardNotPresentException;
using keyple::plugin::pcsc::cpp::exception::CardTerminalException;

using DisconnectionMode = PcscReader::DisconnectionMode;

CardTerminal::CardTerminal(
  const std::shared_ptr<CardTerminals> cardTerminals
, const std::string& name)
: mName(name)
, mCardTerminals(cardTerminals)
{
}

const std::string&
CardTerminal::getName() const
{
    return mName;
}

std::shared_ptr<Card>
CardTerminal::connect(const std::string& protocol)
{
    int dwPreferredProtocols;
    int dwShareMode = SCARD_SHARE_SHARED;

    std::string _protocol = StringUtils::toupper(protocol);

    /* Proprietary extension */
    if (StringUtils::startsWith(protocol, "EXCLUSIVE;")) {
        dwShareMode = SCARD_SHARE_EXCLUSIVE;
        const size_t prefixSize = std::string("EXCLUSIVE;").size();
        _protocol = _protocol.substr(prefixSize, protocol.size() - prefixSize);
    }

    if ("T=0" == _protocol) {
        dwPreferredProtocols = SCARD_PROTOCOL_T0;
    } else if ("T=1" == _protocol) {
        dwPreferredProtocols = SCARD_PROTOCOL_T1;
    } else if ("*" == _protocol) {
        dwPreferredProtocols = SCARD_PROTOCOL_ANY;
    } else if ("DIRECT" == _protocol) {
        /* Connect directly to reader to send control commands. */
        dwPreferredProtocols = 0;
        // /* OSX 10.11 would otherwise fail with SCARD_E_INVALID_VALUE
        // if (Platform.isMac()) {
        //     dwPreferredProtocols = SCARD_PROTOCOL_ANY;
        // }
        dwShareMode = SCARD_SHARE_DIRECT;
    } else {
        throw IllegalArgumentException(
            "Protocol should be one of (prepended with EXCLUSIVE;) T=0, T=1," \
            " *, DIRECT. Got " + protocol);
    }

    DWORD dwProtocol;
    SCARDHANDLE handle;
    SCARD_IO_REQUEST ioRequest;

    LONG rv = SCardConnect(
        mCardTerminals->mContext,
        (LPCSTR)mName.c_str(),
        (DWORD)dwShareMode,
        (DWORD)dwPreferredProtocols,
        &handle,
        &dwProtocol);

    if (rv == SCARD_S_SUCCESS) {
        switch (dwProtocol) {
        case SCARD_PROTOCOL_T0:
            ioRequest = *SCARD_PCI_T0;
            break;
        case SCARD_PROTOCOL_T1:
            ioRequest = *SCARD_PCI_T1;
            break;
        }

        DWORD readerLength;
        BYTE _atr[33];
        DWORD atrLen = sizeof(_atr);
        DWORD state;

        rv = SCardStatus(
            handle,
            NULL,
            &readerLength,
            &state,
            &dwProtocol,
            _atr,
            &atrLen);

        std::vector<uint8_t> atr(_atr, _atr + atrLen);

        return std::make_shared<Card>(
            shared_from_this(), handle, atr, dwProtocol, ioRequest);

    } else if (rv == static_cast<LONG>(SCARD_W_REMOVED_CARD)) {
        throw CardNotPresentException("Card not present.");

    } else {
        throw RuntimeException("Should not reach here.");
    }
}

bool
CardTerminal::isCardPresent()
{
    SCARD_READERSTATE states[1];
    states[0].szReader = mName.c_str();

    LONG rv = SCardGetStatusChange(mCardTerminals->mContext, 0, states, 1);
    if (rv != SCARD_S_SUCCESS) {
        mLogger->error(
            "SCardGetStatusChange failed with error: %\n",
            std::string(pcsc_stringify_error(rv)));
        throw CardException(
            "Failed to get reader status: error " +
            std::string(pcsc_stringify_error(rv)));
    }

	return 0 != (states[0].dwEventState & SCARD_STATE_PRESENT);
}

bool
CardTerminal::waitForCardAbsent(uint64_t timeout)
{
    uint64_t newMs = 0;
    uint64_t currentMs
        = std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count();

    do {
        bool isCardAbsent = !this->isCardPresent();
        if (isCardAbsent) {
            return true;
        }

        newMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

    } while (newMs <= (currentMs + timeout));

    return false;
}

bool
CardTerminal::waitForCardPresent(uint64_t timeout)
{
    uint64_t newMs = 0;
    uint64_t currentMs
        = std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count();

    do {
        bool isCardPresent = this->isCardPresent();
        if (isCardPresent) {
            return true;
        }

        newMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

    } while (newMs <= (currentMs + timeout));

    return false;
}

bool
CardTerminal::operator==(const CardTerminal& o) const
{
    return !mName.compare(o.mName);
}

bool
CardTerminal::operator!=(const CardTerminal& o) const
{
    return !(*this == o);
}

std::ostream&
operator<<(std::ostream& os, const CardTerminal& t)
{
    os << "CardTerminal: {"
       << "NAME = " << t.mName << "}";

    return os;
}

std::ostream&
operator<<(std::ostream& os, const std::vector<CardTerminal>& vt)
{
    os << "CardTerminalS: {";
    for (const auto& t : vt) {
        os << t;
        if (t != vt.back())
            os << ", ";
    }
    os << "}";

    return os;
}

} /* namespace cpp */
} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
