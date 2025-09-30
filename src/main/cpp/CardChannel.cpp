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

#include "keyple/plugin/pcsc/cpp/CardChannel.hpp"

#include "PcscUtils.hpp"

#include "keyple/core/util/cpp/KeypleStd.hpp"
#include "keyple/core/util/cpp/exception/IllegalArgumentException.hpp"
#include "keyple/plugin/pcsc/cpp/Card.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardException.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using keyple::core::util::cpp::exception::IllegalArgumentException;
using keyple::plugin::pcsc::cpp::exception::CardException;

CardChannel::CardChannel(const std::shared_ptr<Card> card, const int channel)
: mChannel(channel)
, mIsClosed(true)
, mCard(card)
{

}

std::shared_ptr<Card>
CardChannel::getCard() const
{
    return mCard;
}

std::vector<uint8_t>
CardChannel::transmit(const std::vector<uint8_t>& apduIn)
{
    if (apduIn.size() == 0)
        throw IllegalArgumentException("command cannot be empty");

    /* Make a copy */
    std::vector<uint8_t> _apduIn = apduIn;

    /* To check */
    bool t0GetResponse = true;
    bool t1GetResponse = true;

    /*
     * Note that we modify the 'command' array in some cases, so it must be
     * a copy of the application provided data
     */
    int n = static_cast<int>(_apduIn.size());
    bool t0 = mCard->mProtocol == SCARD_PROTOCOL_T0;
    bool t1 = mCard->mProtocol == SCARD_PROTOCOL_T1;

    if (t0 && (n >= 7) && (_apduIn[4] == 0))
        throw CardException("Extended len. not supported for T=0");

    if ((t0 || t1) && (n >= 7)) {
        int lc = _apduIn[4] & 0xff;
        if (lc != 0) {
            if (n == lc + 6) {
                n--;
            }
        } else {
            lc = ((_apduIn[5] & 0xff) << 8) | (_apduIn[6] & 0xff);
            if (n == lc + 9) {
                n -= 2;
            }
        }
    }

    bool getresponse = (t0 && t0GetResponse) || (t1 && t1GetResponse);
    int k = 0;
    std::vector<uint8_t> result;

    while (true) {
        if (++k >= 32) {
            throw CardException("Could not obtain response");
        }

        char r_apdu[261];
        DWORD dwRecv = sizeof(r_apdu);
        uint64_t rv;

        mLogger->debug("transmitApdu - c-apdu >> %\n", _apduIn);

        rv = SCardTransmit(
            mCard->mHandle,
            &mCard->mIORequest,
            (LPCBYTE)_apduIn.data(),
            static_cast<DWORD>(_apduIn.size()),
            NULL,
            (LPBYTE)r_apdu,
            &dwRecv);
        if (rv != SCARD_S_SUCCESS) {
            mLogger->error(
                "SCardTransmit failed with error: %\n",
                std::string(pcsc_stringify_error(rv)));
            throw CardException("ScardTransmit failed");
        }

        std::vector<uint8_t> response(r_apdu, r_apdu + dwRecv);

        mLogger->debug("transmitApdu - r-apdu << %\n", response);

        int rn = static_cast<int>(response.size());
        if (getresponse && (rn >= 2)) {
            /* See ISO 7816/2005, 5.1.3 */
            if ((rn == 2) && (response[0] == 0x6c)) {
                // Resend command using SW2 as short Le field
                _apduIn[n - 1] = response[1];
                continue;
            }

            if (response[rn - 2] == 0x61) {
                /* Issue a GET RESPONSE command with the same CLA using SW2
                 * as short Le field */
                if (rn > 2)
                    result.insert(
                        result.end(),
                        response.begin(),
                        response.begin() + rn - 2);

                std::vector<uint8_t> getResponse(5);
                getResponse[0] = _apduIn[0];
                getResponse[1] = 0xC0;
                getResponse[2] = 0;
                getResponse[3] = 0;
                getResponse[4] = response[rn - 1];
                n = 5;
                _apduIn.swap(getResponse);
                continue;
            }
        }

        result.insert(result.end(), response.begin(), response.begin() + rn);
        break;
    }

    return result;
}

} /* namespace cpp */
} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */