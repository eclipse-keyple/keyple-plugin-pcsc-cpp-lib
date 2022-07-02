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

#include "CardTerminal.h"

#include <chrono>
#include <cstring>

/* Kepyle Core Util */
#include "IllegalArgumentException.h"
#include "KeypleStd.h"
#include "System.h"

/* Keyple Core Service */
#include "Thread.h"

/* PC/SC plugin */
#include "CardTerminalException.h"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::plugin::pcsc::cpp::exception;

using DisconnectionMode = PcscReader::DisconnectionMode;

#ifdef WIN32
std::string pcsc_stringify_error(LONG rv)
{
    static char out[20];
    sprintf_s(out, sizeof(out), "0x%08X", rv);

    return std::string(out);
}
#endif

CardTerminal::CardTerminal(const std::string& name)
: mContext(0), mHandle(0), mState(0), mName(name), mContextEstablished(false)
{
    memset(&mPioSendPCI, 0, sizeof(SCARD_IO_REQUEST));
}

const std::string& CardTerminal::getName() const
{
    return mName;
}

const std::vector<std::string>& CardTerminal::listTerminals()
{
    ULONG ret;
    SCARDCONTEXT context;
    char* readers = NULL;
    char* ptr     = NULL;
    DWORD len     = 0;
    static std::vector<std::string> list;

    /* Clear list */
    list.clear();

    ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &context);
    if (ret != SCARD_S_SUCCESS) {
        throw CardTerminalException("SCardEstablishContext failed");
    }

    ret = SCardListReaders(context, NULL, NULL, &len);
    if (ret != SCARD_S_SUCCESS && ret != SCARD_E_NO_READERS_AVAILABLE) {
        throw CardTerminalException("SCardListReaders failed");
    }

    if (ret == SCARD_E_NO_READERS_AVAILABLE || len == 0) {
        /* No readers to add to list */
        return list;
    }

    readers = (char*)calloc(len, sizeof(char));

    if (readers == NULL) {
        /* No readers to add to list */
        return list;
    }

    ret = SCardListReaders(context, NULL, readers, &len);
    if (ret != SCARD_S_SUCCESS) {
        throw CardTerminalException("SCardListReaders failed");
    }

    ptr = readers;

    if (!ptr) {
        return list;
    }

    while (*ptr) {
        std::string s(ptr);
        list.push_back(s);
        ptr += strlen(ptr) + 1;
    }

    SCardReleaseContext(context);
    free(readers);

    return list;
}

void CardTerminal::establishContext()
{
    if (mContextEstablished)
        return;

    LONG ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &mContext);
    if (ret != SCARD_S_SUCCESS) {
        mContextEstablished = false;
        mLogger->error("SCardEstablishContext failed with error: %\n",
                     std::string(pcsc_stringify_error(ret)));
        throw CardTerminalException("SCardEstablishContext failed");
    }

    mContextEstablished = true;
}

void CardTerminal::releaseContext()
{
    if (!mContextEstablished)
        return;

    SCardReleaseContext(mContext);
    mContextEstablished = false;
}

bool CardTerminal::connect()
{
    LONG rv = SCardConnect(mContext,
                           (LPCSTR)mName.c_str(),
                           SCARD_SHARE_SHARED,
                           SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                           &mHandle,
                           &mProtocol);

    return rv == SCARD_S_SUCCESS;
}

void CardTerminal::disconnect()
{
    SCardDisconnect(mHandle, SCARD_LEAVE_CARD);
}

bool CardTerminal::isCardPresent(bool release)
{
    (void)release;
    try {
        establishContext();
    } catch (CardTerminalException& e) {
        mLogger->error("isCardPresent - caught CardTerminalException %\n", e);
        throw;
    }

    bool status = connect();
    disconnect();

    return status;
}

void CardTerminal::openAndConnect(const std::string& protocol)
{
    LONG rv;
    DWORD connectProtocol;
    DWORD sharingMode = SCARD_SHARE_SHARED;
    BYTE reader[200];
    DWORD readerLen = sizeof(reader);
    BYTE _atr[33];
    DWORD atrLen = sizeof(_atr);

    mLogger->debug("[%] openAndConnect - protocol: %\n", mName, protocol);

    try {
        establishContext();
    } catch (CardTerminalException& e) {
        mLogger->error("openAndConnect - caught CardTerminalException %\n", e);
        throw;
    }

    if (!protocol.compare("*")) {
        connectProtocol = SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
    } else if (!protocol.compare("T=0")) {
        connectProtocol = SCARD_PROTOCOL_T0;
    } else if (!protocol.compare("T=1")) {
        connectProtocol = SCARD_PROTOCOL_T1;
    } else if (!protocol.compare("direct")) {
        connectProtocol = 0;
        sharingMode     = SCARD_SHARE_DIRECT;
    } else {
        throw IllegalArgumentException("Unsupported protocol " + protocol);
    }

    mLogger->debug("openAndConnect - connecting tp % with protocol: %, "
                   "connectProtocol: % and sharingMode: %\n",
                   mName,
                   protocol,
                   connectProtocol,
                   sharingMode);

    rv = SCardConnect(mContext,
                      mName.c_str(),
                      sharingMode,
                      connectProtocol,
                      &mHandle,
                      &mProtocol);
    if (rv != SCARD_S_SUCCESS) {
        mLogger->error("openAndConnect - SCardConnect failed (%)\n",
                       std::string(pcsc_stringify_error(rv)));
        releaseContext();
        throw CardTerminalException("openAndConnect failed");
    }

    switch (mProtocol) {
    case SCARD_PROTOCOL_T0:
        mPioSendPCI = *SCARD_PCI_T0;
        break;
    case SCARD_PROTOCOL_T1:
        mPioSendPCI = *SCARD_PCI_T1;
        break;
    }

    rv = SCardStatus(mHandle, (LPSTR)reader, &readerLen, &mState,
                     &mProtocol, _atr, &atrLen);
    if (rv != SCARD_S_SUCCESS) {
        mLogger->error("openAndConnect - SCardStatus failed (s)\n",
                      std::string(pcsc_stringify_error(rv)));
        releaseContext();
        throw CardTerminalException("openAndConnect failed");
    } else {
        mLogger->debug("openAndConnect - card state: %\n", mState);
    }

    mAtr.clear();
    mAtr.insert(mAtr.end(), _atr, _atr + atrLen);
}

void CardTerminal::closeAndDisconnect(const DisconnectionMode mode)
{
    mLogger->debug("[%] closeAndDisconnect - mode: %\n", mName, mode);

    SCardDisconnect(mContext, mode == DisconnectionMode::RESET ? SCARD_RESET_CARD : SCARD_LEAVE_CARD);

    releaseContext();
}

const std::vector<uint8_t>& CardTerminal::getATR()
{
    return mAtr;
}

std::vector<uint8_t> CardTerminal::transmitApdu(const std::vector<uint8_t>& apduIn)
{
    if (apduIn.size() == 0)
        throw IllegalArgumentException("command cannot be empty");

    /* Make a copy */
    std::vector<uint8_t> _apduIn = apduIn;

    /* To check */
    bool t0GetResponse = true;
    bool t1GetResponse = true;
    bool t1StripLe     = true;

    /*
     * Note that we modify the 'command' array in some cases, so it must be a copy of the
     * application provided data
     */
    int n   = static_cast<int>(_apduIn.size());
    bool t0 = mProtocol == SCARD_PROTOCOL_T0;
    bool t1 = mProtocol == SCARD_PROTOCOL_T1;

    if (t0 && (n >= 7) && (_apduIn[4] == 0))
        throw CardTerminalException("Extended len. not supported for T=0");

    if ((t0 || (t1 && t1StripLe)) && (n >= 7)) {
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
            throw CardTerminalException("Could not obtain response");
        }

        char r_apdu[261];
        DWORD dwRecv = sizeof(r_apdu);
        long rv;

        mLogger->debug("[%] transmitApdu - c-apdu >> %\n", mName, _apduIn);

        rv = SCardTransmit(mHandle,
                           &mPioSendPCI,
                           (LPCBYTE)_apduIn.data(),
                           static_cast<DWORD>(_apduIn.size()),
                           NULL,
                           (LPBYTE)r_apdu,
                           &dwRecv);
        if (rv != SCARD_S_SUCCESS) {
            mLogger->error("SCardTransmit failed with error: %\n",
                           std::string(pcsc_stringify_error(rv)));
            throw CardTerminalException("ScardTransmit failed");
        }

        std::vector<uint8_t> response(r_apdu, r_apdu + dwRecv);

        mLogger->debug("[%] transmitApdu - r-apdu << %\n", mName, response);

        int rn = static_cast<int>(response.size());
        if (getresponse && (rn >= 2)) {
            /* See ISO 7816/2005, 5.1.3 */
            if ((rn == 2) && (response[0] == 0x6c)) {
                // Resend command using SW2 as short Le field
                _apduIn[n - 1] = response[1];
                continue;
            }

            if (response[rn - 2] == 0x61) {
                /* Issue a GET RESPONSE command with the same CLA using SW2 as short Le field */
                if (rn > 2)
                    result.insert(result.end(), response.begin(), response.begin() + rn - 2);

                std::vector<uint8_t> getResponse(5);
                getResponse[0] = _apduIn[0];
                getResponse[1] = 0xC0;
                getResponse[2] = 0;
                getResponse[3] = 0;
                getResponse[4] = response[rn - 1];
                n          = 5;
                _apduIn.swap(getResponse);
                continue;
            }
        }

        result.insert(result.end(), response.begin(), response.begin() + rn);
        break;
    }

    return result;
}

void CardTerminal::beginExclusive()
{
}

void CardTerminal::endExclusive()
{
}

bool CardTerminal::waitForCardAbsent(long timeout)
{
    uint64_t newMs = 0;
    uint64_t currentMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch()).count();

    do {
        bool isCardAbsent = !this->isCardPresent(false);
        if (isCardAbsent) {
            return true;
        }

        newMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch()).count();

    } while (newMs <= (currentMs + timeout));

    return false;
}

bool CardTerminal::waitForCardPresent(long timeout)
{
    uint64_t newMs = 0;
    uint64_t currentMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch()).count();

    do {
        bool isCardPresent = this->isCardPresent(false);
        if (isCardPresent) {
            return true;
        }

        newMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch()).count();

    } while (newMs <= (currentMs + timeout));

    return false;
}

bool CardTerminal::operator==(const CardTerminal& o) const
{
    return !mName.compare(o.mName);
}

bool CardTerminal::operator!=(const CardTerminal& o) const
{
	return !(*this == o);
}

std::ostream& operator<<(std::ostream& os, const CardTerminal& t)
{
    os << "CardTerminal: {"
       << "NAME = " << t.mName
	   << "}";

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<CardTerminal>& vt)
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

}
}
}
}
