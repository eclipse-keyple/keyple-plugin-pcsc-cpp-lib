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
#include <vector>

#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
#include <winscard.h>
#else
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#endif

#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;

class Card;

/**
 * A logical channel connection to a Smart Card. It is used to exchange APDUs
 * with a Smart Card. A CardChannel object can be obtained by calling the method
 * Card::getBasicChannel() or Card::openLogicalChannel().
 */
class CardChannel {
public:
    /**
     *
     */
    CardChannel(const std::shared_ptr<Card> card, const int channel);

    /**
     * Returns the Card this channel is associated with.
     *
     * @return The Card this channel is associated with.
     */
    std::shared_ptr<Card>
    getCard() const;

    /**
     * Returns the channel number of this CardChannel. A channel number of 0
     * indicates the basic logical channel.
     *
     * @throw IllegalStateException If this channel has been closed or if the
     * corresponding Card has been disconnected.
     * @return The channel number of this CardChannel.
     */
    int
    getChannelNumber() const;

    /**
     * Closes this CardChannel. The logical channel is closed by issuing a
     * MANAGE CHANNEL command that should use the format [xx 70 80 0n] where n
     * is the channel number of this channel and xx is the CLA byte that encodes
     * this logical channel and has all other bits set to 0. After this method
     * returns, calling other methods in this class will raise an
     * IllegalStateException.
     *
     * Note that the basic logical channel cannot be closed using this method.
     * It can be closed by calling Card.disconnect(boolean).
     *
     * @throw CardException If the card operation failed.
     * @throw IllegalStateException If this CardChannel represents a connection
     * the basic logical channel
     */
    void
    close();

    /**
     * Transmits the command APDU stored in the command apduIn and receives
     * the response APDU in the response.
     *
     * @param apduIn C-APDU
     * @return R-APDU in the form of a byte vector.
     */
    std::vector<uint8_t> transmit(const std::vector<uint8_t>& apduIn);

private:
    /**
     *
     */
    const std::shared_ptr<Logger> mLogger =
        LoggerFactory::getLogger(typeid(CardChannel));

    /**
     *
     */
    int mChannel;

    /**
     *
     */
    bool mIsClosed;

    /**
     *
     */
    std::shared_ptr<Card> mCard;
};

} /* namespace cpp */
} /* namespace pcsc*/
} /* namespace pcsc */
} /* namespace plugin */