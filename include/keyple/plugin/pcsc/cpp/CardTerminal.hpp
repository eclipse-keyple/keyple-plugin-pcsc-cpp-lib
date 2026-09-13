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

#pragma once

#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
#include <winscard.h>
#else
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#endif

#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/plugin/pcsc/KeyplePluginPcscExport.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/cpp/Card.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;

using DisconnectionMode = PcscReader::DisconnectionMode;

class CardTerminals;

class KEYPLEPLUGINPCSC_API CardTerminal
: public std::enable_shared_from_this<CardTerminal> {
public:
    /**
     *
     */
    explicit CardTerminal(
        std::shared_ptr<CardTerminals> cardTerminals, const std::string& name);

    /**
     *
     */
    virtual ~CardTerminal() = default;

    /**
     * Returns the unique name of this terminal.
     *
     * @return the unique name of this terminal.
     */
    const std::string&
    getName() const;

    /**
     * Returns whether a card is present in this terminal.
     *
     * @return whether a card is present in this terminal.
     * @throw CardException if the status could not be determined.
     */
    bool
    isCardPresent();

    /**
     * Establishes a connection to the card. If a connection has previously
     * established using the specified protocol, this method returns the same
     * Card object as the previous call.
     *
     * @param protocol The protocol to use ("T=0", "T=1", or "T=CL"), or "*" to
     * connect using any available protocol.
     * @throw IllegalArgumentException if protocol is an invalid protocol
     * specification.
     * @throw CardNotPresentException If no card is present in this terminal.
     * @throw CardException If a connection could not be established using the
     * specified protocol or if a connection has previously been established
     * using a different protocol.
     * @throw SecurityException If a SecurityManager exists and the caller does
     * not have the required permission.
     */
    std::shared_ptr<Card> connect(const std::string& protocol);

    /**
     * Waits until a card is absent in this terminal or the timeout
     * expires. If the method returns due to an expired timeout, it returns
     * false. Otherwise it return true.
     *
     * <P>If no card is present in this terminal when this
     * method is called, it returns immediately.
     *
     * @param timeout if positive, block for up to <code>timeout</code>
     * milliseconds; if zero, block indefinitely; must not be negative
     * @return false if the method returns due to an expired timeout,
     * true otherwise.
     * @throw IllegalArgumentException if timeout is negative
     * @throw CardException if the operation failed
     */
    bool
    waitForCardAbsent(uint64_t timeout);

    /**
     * Waits until a card is present in this terminal or the timeout
     * expires. If the method returns due to an expired timeout, it returns
     * false. Otherwise it return true.
     *
     * <P>If a card is present in this terminal when this
     * method is called, it returns immediately.
     *
     * @param timeout if positive, block for up to <code>timeout</code>
     *   milliseconds; if zero, block indefinitely; must not be negative
     * @return false if the method returns due to an expired timeout,
     *   true otherwise.
     * @throw IllegalArgumentException if timeout is negative
     * @throw CardException if the operation failed
     */
    bool
    waitForCardPresent(uint64_t timeout);

	/**
	 *
	 */
	friend std::ostream&
    operator<<(std::ostream& os, const CardTerminal& t);

	/**
	 *
	 */
    friend std::ostream&
    operator<<(std::ostream& os, const std::vector<CardTerminal>& vt);

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
    const std::string mName;

    /**
     *
     */
    const std::shared_ptr<CardTerminals> mCardTerminals;
};

} /* namespace cpp */
} /* namespace pcsc*/
} /* namespace pcsc */
} /* namespace plugin */
