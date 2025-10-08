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

#include "keyple/plugin/pcsc/cpp/Card.hpp"

#include "PcscUtils.hpp"

#include "keyple/plugin/pcsc/cpp/exception/CardException.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using keyple::plugin::pcsc::cpp::exception::CardException;

Card::Card(
  const std::shared_ptr<CardTerminal> cardTerminal,
  const SCARDHANDLE handle,
  const std::vector<uint8_t> atr,
  const DWORD protocol,
  const SCARD_IO_REQUEST ioRequest)
: mProtocol(protocol)
, mIORequest(ioRequest)
, mHandle(handle)
, mAtr(atr)
, mCardTerminal(cardTerminal)
{

}

const std::vector<uint8_t>&
Card::getATR() const
{
    return mAtr;
}

void
Card::beginExclusive()
{
    SCardBeginTransaction(mHandle);
}

void
Card::endExclusive()
{
    SCardEndTransaction(mHandle, SCARD_LEAVE_CARD);
}

void
Card::disconnect(const bool reset)
{
    SCardDisconnect(mHandle, reset ? SCARD_RESET_CARD : SCARD_LEAVE_CARD);
}

const std::string
Card::getProtocol() const
{
    switch (mProtocol) {
    case SCARD_PROTOCOL_T0:
        return "T=0";
    case SCARD_PROTOCOL_T1:
        return "T=1";
    default:
        return "DIRECT";
    }
}

std::shared_ptr<CardChannel>
Card::getBasicChannel()
{
    return std::make_shared<CardChannel>(shared_from_this(), 0);
}

const std::vector<uint8_t>
Card::transmitControlCommand(
    const int commandId, const std::vector<uint8_t>& command)
{
    char r_apdu[261];
    DWORD dwRecv = sizeof(r_apdu);

    LONG rv = SCardControl(
        mHandle,
        (DWORD)commandId,
        (LPCBYTE)command.data(),
        (DWORD)command.size(),
        (LPBYTE)r_apdu,
        (DWORD)sizeof(r_apdu),
        &dwRecv);
    if (rv != SCARD_S_SUCCESS) {
        mLogger->error(
            "SCardControl failed with error: %\n",
            std::string(pcsc_stringify_error(rv)));
        throw CardException("SCardControl failed");
    }

    std::vector<uint8_t> response(r_apdu, r_apdu + dwRecv);

    return response;
}

} /* namespace cpp */
} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */