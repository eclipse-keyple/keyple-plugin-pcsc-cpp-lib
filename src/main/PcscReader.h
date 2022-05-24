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

#include <ostream>
#include <string>

/* Keyple Core Common */
#include "KeypleReaderExtension.h"

/* Keyple Plugin Pcsc */
#include "KeyplePluginPcscExport.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::common;

/**
 * PC/SC specific {@link KeypleReaderExtension}.
 *
 * <p>Provides specific settings and methods for configuring a PC/SC reader.
 *
 * @since 2.0.0
 */
class KEYPLEPLUGINPCSC_API PcscReader : public KeypleReaderExtension {
public:
    /**
     * Connection mode to indicate if the application is willing to share the card with other
     * applications.
     *
     * <p>Corresponds to the beginExclusive() and endExclusive() methods of smartcard.io and, at a
     * lower level, to the connection mode defined by PC/SC and used in the SCardConnect function.
     *
     * @since 2.0.0
     */
    enum class KEYPLEPLUGINPCSC_API SharingMode {
        /**
         * Allows simultaneous access to the card
         *
         * @since 2.0.0
         */
        SHARED,

        /**
         * Requests exclusive access to the card
         *
         * @since 2.0.0
         */
        EXCLUSIVE
    };

    /**
     * Available transmission protocols as defined by the PC/SC standard.
     *
     * @since 2.0.0
     */
    class KEYPLEPLUGINPCSC_API IsoProtocol {
    public:
        /**
         * to connect using any available protocol
         *
         * @since 2.0.0
         */
        static const IsoProtocol ANY;
        
        /**
         * to connect using T=0 protocol
         *
         * @since 2.0.0
         */
        static const IsoProtocol T0;

        /**
         * to connect using T=1 protocol
         *
         * @since 2.0.0
         */
        static const IsoProtocol T1;

        /**
         * to connect using T=CL protocol
         *
         * @since 2.0.0
         */
        static const IsoProtocol TCL;

        /**
         * Constructor.
         *
         * <p>Associates the enum value with its corresponding definition in the PC/SC standard.
         *
         * @param value A string
         * @since 2.0.0
         */
        IsoProtocol(const std::string& value);

        /**
         * Gets the string expected by smartcard.io / PC/SC to set the card transmission protocol.
         *
         * @return A not empty string.
         * @since 2.0.0
         */
        const std::string& getValue() const;

        /**
         * 
         */
        bool operator==(const PcscReader::IsoProtocol& ip) const;

    private:
        /**
         * 
         */
        const std::string mValue;
    };

    /**
     * Action to be taken after the card is disconnected.
     *
     * @since 2.0.0
     */
    enum class DisconnectionMode {
        /**
         * Resets the card
         *
         * @since 2.0.0
         */
        RESET,

        /**
         * Keeps the status of the card unchanged
         *
         * @since 2.0.0
         */
        LEAVE
    };

    /**
     * 
     */
    virtual ~PcscReader() = default;

    /**
     * Changes the PC/SC sharing mode (default value {@link SharingMode#EXCLUSIVE}).
     *
     * <p>This mode will be used when a new {@link Card} is created.
     *
     * <p>If a card is already inserted, changes immediately the mode in the current {@link Card}
     * object.
     *
     * @param sharingMode The {@link SharingMode} to use (must be not null).
     * @return This instance.
     * @throws IllegalArgumentException If sharingMode is null
     * @throws IllegalStateException If the sharing mode setting failed.
     * @since 2.0.0
     */
    virtual PcscReader& setSharingMode(const SharingMode sharingMode) = 0;

    /**
     * Sets the reader transmission mode.
     *
     * <p>A PC/SC reader can be contact or contactless. There is no way by generic programming to
     * know what type of technology a reader uses.
     *
     * <p>Thus, it is the responsibility of the application to give the reader the means to know his
     * own type.
     *
     * <p>This can be done in two ways:
     *
     * <ul>
     *   <li>by invoking this method,
     *   <li>by giving the plugin the means to determine the type from the name of the reader. In
     *       the latter case, the application does not need to call this method, the reader will
     *       determine its type itself using the plugin parameters (see PcscPluginFactoryBuilder.
     * </ul>
     *
     * <p>There is no default mode, so it is mandatory to set the mode by any of the above methods.
     *
     * @param contactless true to set contactless mode, false to set contact mode.
     * @return This instance.
     * @since 2.0.0
     */
    virtual PcscReader& setContactless(const bool contactless) = 0;

    /**
     * Changes the protocol to be used by the PC/SC reader when connecting to the card ({@link
     * IsoProtocol#T0}, {@link IsoProtocol#T1}, or {@link IsoProtocol#TCL}), or {@link
     * IsoProtocol#ANY} to connect using any available protocol (default value {@link
     * IsoProtocol#ANY}).
     *
     * @param isoProtocol The {@link IsoProtocol} to use (must be not null).
     * @return This instance.
     * @throw IllegalArgumentException If isoProtocol is null
     * @since 2.0.0
     */
    virtual PcscReader& setIsoProtocol(const IsoProtocol& isoProtocol) = 0;

    /**
     * Changes the action to be taken after disconnection (default value {@link
     * DisconnectionMode#LEAVE}).
     *
     * <p>The card is either reset or left as is.
     *
     * <p>The default value for this parameter if this method is not called is {@link
     * DisconnectionMode#LEAVE}.
     *
     * @param disconnectionMode The {@link DisconnectionMode} to use (must be not null).
     * @return This instance.
     * @throws IllegalArgumentException If disconnectionMode is null
     */
    virtual PcscReader& setDisconnectionMode(const DisconnectionMode disconnectionMode) = 0;

    /**
     * 
     */
    friend std::ostream& operator<<(std::ostream& os, const SharingMode sm);

    /**
     * 
     */
    friend std::ostream& operator<<(std::ostream& os, const IsoProtocol& ip);

    /**
     * 
     */
    friend std::ostream& operator<<(std::ostream& os, const DisconnectionMode dm);
};

}
}
}