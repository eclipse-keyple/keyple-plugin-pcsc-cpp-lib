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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
#include <winscard.h>
#else
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#endif

#include "keyple/plugin/pcsc/cpp/CardChannel.hpp"
#include "keyple/plugin/pcsc/cpp/CardChannel.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;

class CardTerminal;

/**
 * A Smart Card with which a connection has been established. Card objects are
 * obtained by calling CardTerminal.connect().
 */
class Card : public std::enable_shared_from_this<Card> {
public:
    /**
     *
     */
    DWORD mProtocol;

    /**
     *
     */
    const SCARD_IO_REQUEST mIORequest;

    /**
     *
     */
    const SCARDHANDLE mHandle;

    /**
     * Constructor.
     */
    Card(
        const std::shared_ptr<CardTerminal> cardTerminal,
        const SCARDHANDLE handle,
        const std::vector<uint8_t> atr,
        const DWORD protocol,
        const SCARD_IO_REQUEST ioRequest);

    /**
     * Destructor.
     */
    virtual ~Card() = default;

    /**
     * Disconnects the connection with this card. After this method returns,
     * calling methods on this object or in CardChannels associated with this
     * object that require interaction with the card will raise a
     * IllegalStateException.
     *
     * @param reset Whether to reset the card after disconnecting.
     * @throw CardException If the card operation failed.
     * @throw SecurityException If a SecurityManager exists and the caller doe
     * not have the required permission.
     */
    void
    disconnect(const bool reset);

    /**
     * Returns the ATR of this card.
     *
     * @return the ATR of this card.
     */
    const std::vector<uint8_t>&
    getATR() const;

    /**
     * Requests exclusive access to this card.
     *
     * Once a thread has invoked beginExclusive, only this thread is allowed to
     * communicate with this card until it calls endExclusive. Other threads
     * attempting communication will receive a CardException.
     *
     * @throw CardException If exclusive access has already been set or if
     * exclusive access could not be established
     * @throw IllegalStateException If this card object has been disposed of via
     * the disconnect() method
     */
    void
    beginExclusive();

    /**
     * Releases the exclusive access previously established using
     * beginExclusive.
     *
     * @throw CardException If the CardException - if the
     * @throw IllegalStateException If the active Thread does not currently have
     * exclusive access to this card or if this card object has been disposed of
     * via the disconnect() method
     */
    void
    endExclusive();

    /**
     * Returns the protocol in use for this card.
     */
    const std::string
    getProtocol() const;

    /**
     * Returns the CardChannel for the basic logical channel. The basic logical
     * channel has a channel number of 0.
     *
     * @throw IllegalStateException If this card object has been disposed of via
     * the disconnect() method
     */
    std::shared_ptr<CardChannel>
    getBasicChannel();

    /**
     *Transmits a control command to the terminal device.
     *
     * @param controlCode The control code of the command
     * @param command The command data
     * @throw CardException If the card operation failed
     * @throw IllegalStateException If this card object has been disposed of via
     * the disconnect() method
     */
    virtual const std::vector<uint8_t>
    transmitControlCommand(
        const int commandId, const std::vector<uint8_t>& command);

private:
    /**
     *
     */
    const std::shared_ptr<Logger> mLogger =
        LoggerFactory::getLogger(typeid(Card));

    /**
     *
     */
    std::vector<uint8_t> mAtr;

    /**
     *
     */
    const std::shared_ptr<CardTerminal> mCardTerminal;
};

} /* namespace cpp */
} /* namespace pcsc*/
} /* namespace pcsc */
} /* namespace plugin */