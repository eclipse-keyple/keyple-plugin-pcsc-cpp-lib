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

#include <memory>
#include <string>

#include "keyple/plugin/pcsc/cpp/CardTerminal.hpp"

#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
#include <winscard.h>
#else
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#endif

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

/**
 * The set of terminals supported by a TerminalFactory.
 * This class allows applications to enumerate the available CardTerminals,
 * obtain a specific CardTerminal, or wait for the insertion or removal of
 * cards.
 */
class CardTerminals : public std::enable_shared_from_this<CardTerminals> {
public:
    /**
     *
     */
    SCARDCONTEXT& mContext;

    /**
     * Enumeration of attributes of a CardTerminal.
     * It is used as a parameter to the {@linkplain CardTerminals#list} method.
     *
     * @since 1.6
     */
    enum class State {
        /**
         * All CardTerminals.
         */
        ALL,

        /**
         * CardTerminals in which a card is present.
         */
        CARD_PRESENT,

        /**
         * CardTerminals in which a card is not present.
         */
        CARD_ABSENT,

        /**
         * CardTerminals for which a card insertion was detected during the
         * latest call to waitForChange()
         */
        CARD_INSERTION,

        /**
         * CardTerminals for which a card removal was detected during the
         * latest call to waitForChange().
         */
        CARD_REMOVAL
    };

    /**
     * Constructs.
     */
    CardTerminals(SCARDCONTEXT& context);

    /**
     * Returns an unmodifiable list of all available terminals.
     *
     * @return an unmodifiable list of all available terminals.
     * @throw CardException if the card operation failed
     */
    const std::vector<std::shared_ptr<CardTerminal>> list();

    /**
     * Returns an unmodifiable list of all terminals matching the specified
     * state.
     *
     * @param state the State
     * @return an unmodifiable list of all terminals matching the specified
     * attribute.
     * @throw NullPointerException if attr is null
     * @throw CardException if the card operation failed
     */
    const std::vector<std::shared_ptr<CardTerminal>> list(const State state);

    /**
     * Returns the terminal with the specified name or null if no such
     * terminal exists.
     *
     * @return the terminal with the specified name or null if no such
     * terminal exists.
     *
     * @throws NullPointerException if name is null
     */
    std::shared_ptr<CardTerminal> getTerminal(const std::string& name);

    /**
     * Waits for card insertion or removal in any of the terminals of this
     * object.
     *
     * @throw IllegalStateException if this <code>CardTerminals</code>
     * object does not contain any terminals
     * @throw CardException if the card operation failed
     */
    void
    waitForChange();

    /**
     * Waits for card insertion or removal in any of the terminals of this
     * object or until the timeout expires.
     *
     * @param timeout if positive, block for up to <code>timeout</code>
     *  milliseconds; if zero, block indefinitely; must not be negative
     * @return false if the method returns due to an expired timeout,
     *  true otherwise.
     * @throw IllegalStateException if this <code>CardTerminals</code>
     * object does not contain any terminals
     * @throw IllegalArgumentException if timeout is negative
     * @throw CardException if the card operation failed
     */
    bool
    waitForChange(long timeout);

private:
    /**
     *
     */
    std::vector<SCARD_READERSTATE> mKnownReaders;

    /**
     *
     */
    std::vector<SCARD_READERSTATE> mZombieReaders;
};

} /* namespace cpp */
} /* namespace pcsc*/
} /* namespace pcsc */
} /* namespace plugin */
