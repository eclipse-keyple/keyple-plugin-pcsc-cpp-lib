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

#pragma once

/* Keyple Core Util */
#include "LoggerFactory.h"

/* Keyple Plugin Pcsc */
#include "KeyplePluginPcscExport.h"
#include "PcscReader.h"

/* PC/SC */
#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
#include <winscard.h>
#else
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#endif

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using namespace keyple::core::util::cpp;

using DisconnectionMode = PcscReader::DisconnectionMode;

class KEYPLEPLUGINPCSC_API CardTerminal {
public:
    /**
     *
     */
    explicit CardTerminal(const std::string& name);

    /**
     *
     */
    const std::string& getName() const;

    /**
     *
     */
    bool isCardPresent(bool release);

    /**
     *
     */
    void openAndConnect(const std::string& protocol);

    /**
     *
     */
    void closeAndDisconnect(const DisconnectionMode mode);

    /**
     *
     */
    static const std::vector<std::string>& listTerminals();

    /**
     *
     */
    const std::vector<uint8_t>& getATR();

    /**
     *
     */
    std::vector<uint8_t> transmitApdu(const std::vector<uint8_t>& apduIn);

    /**
     *
     */
    void beginExclusive();

    /**
     *
     */
    void endExclusive();

    /**
     *
     */
    bool waitForCardAbsent(long timeout);

    /**
     *
     */
    bool waitForCardPresent(long timeout);

	/**
	 *
	 */
	friend std::ostream& operator<<(std::ostream& os, const CardTerminal& t);

	/**
	 *
	 */
    friend std::ostream& operator<<(std::ostream& os, const std::vector<CardTerminal>& vt);

	/**
	 *
	 */
	bool operator==(const CardTerminal& o) const;

	/**
	 *
	 */
	bool operator!=(const CardTerminal& o) const;

private:
    /**
     *
     */
    const std::shared_ptr<Logger> mLogger =
        LoggerFactory::getLogger(typeid(CardTerminal));

    /**
     *
     */
    SCARDCONTEXT mContext;

    /**
     *
     */
    SCARDHANDLE mHandle;

    /**
     *
     */
    DWORD mProtocol;

    /**
     *
     */
    SCARD_IO_REQUEST mPioSendPCI;

    /**
     *
     */
    DWORD mState;

    /**
     *
     */
    const std::string mName;

    /**
     *
     */
    std::vector<uint8_t> mAtr;

    /**
     *
     */
    bool mContextEstablished;

    /**
     *
     */
    void establishContext();

    /**
     *
     */
    void releaseContext();

    /**
     *
     */
    bool connect();

    /**
     *
     */
    void disconnect();
};

}
}
}
}
