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
#include <ostream>
#include <string>
#include <vector>

#include "keyple/core/common/KeypleReaderExtension.hpp"
#include "keyple/plugin/pcsc/KeyplePluginPcscExport.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::common::KeypleReaderExtension;

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
         * to connect using direct protocol
         *
         * @since 2.4.2
         */
        static const IsoProtocol DIRECT;

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
         * Resets the card. This sends a reset signal to the card while keeping
         * the connection alive.
         *
         * <p>Corresponds to PC/SC `SCARD_RESET_CARD`.
         * @since 2.0.0
         */
        RESET,

        /**
         * Leaves the card in its current state without performing any reset or
         * power down.
         *
         * <p>Corresponds to PC/SC `SCARD_LEAVE_CARD`.
         *
         * @since 2.0.0
         */
        LEAVE,

        /**
         * Completely powers off the card.
         *
         * <p>Corresponds to PC/SC `SCARD_UNPOWER_CARD`.
         *
         * <p>This mode is only available with the default security provider.
         * Depending on the provider used, a runtime error may occur during
         * eader enumeration.
         *
         * @since 2.5.0
         */
        UNPOWER,

        /**
         * Ejects the card (if supported by the reader).
         *
         * <p>Corresponds to PC/SC `SCARD_EJECT_CARD`.
         *
         * <p>This mode is only available with the default security provider.
         * Depending on the provider used, a runtime error may occur during
         * reader enumeration.
         *
         * @since 2.5.0
         */
        EJECT
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
     * <p>The default mode is the name-based determination performed by the plugin. See
     * PcscPluginFactoryBuilder::Builder#useContactlessReaderIdentificationFilter(std::string).
     *
     * @param contactless true to set contactless mode, false to set contact mode.
     * @return This instance.
     * @since 2.0.0
     */
    virtual PcscReader& setContactless(const bool contactless) = 0;

    /**
     * Changes the protocol to be used by the PC/SC reader when connecting to the card (
     * IsoProtocol::T0, IsoProtocol::T1, or IsoProtocol::TCL), or
     * IsoProtocol::ANY to connect using any available protocol (default value
     * IsoProtocol::ANY).
     *
     * @param isoProtocol The {@link IsoProtocol} to use (must be not null).
     * @return This instance.
     * @throw IllegalArgumentException If isoProtocol is null
     * @since 2.0.0
     */
    virtual PcscReader& setIsoProtocol(const IsoProtocol& isoProtocol) = 0;

    /**
     * Changes the action to be taken after disconnection (default value
     * DisconnectionMode::RESET).
     *
     * <p>The card is either reset or left as is.
     *
     * @param disconnectionMode The DisconnectionMode to use (must be not null).
     * @return This instance.
     * @throw IllegalArgumentException If disconnectionMode is null
     * @since 2.0.0
     */
    virtual PcscReader& setDisconnectionMode(const DisconnectionMode disconnectionMode) = 0;

    /**
     * Transmits a control command to the terminal device.
     *
     * <p>This can be used to access specific features of the reader such as setting parameters,
     * controlling LEDs, a buzzer or any other proprietary function defined by the reader
     * manufacturer.
     *
     * <p>The supplied command identifier is internally converted into a control code expected by
     * the current platform. Its actual value differs if the platform is Windows.
     *
     * @param commandId The command identifier.
     * @param command A not null byte array containing the command data.
     * @return The response data.
     * @throw IllegalStateException If the communication with the reader has failed.
     * @since 2.1.0
     */
    virtual const std::vector<uint8_t> transmitControlCommand(
        const int commandId, const std::vector<uint8_t>& command) = 0;

    /**
     * Helper method that return the PC/SC IOCTL CCID "Escape" command identifier.
     *
     * <p>The PC/SC IOCTL CCID "Escape" command exists for all platforms but the value of its
     * identifier differs from one to another (3500 for Windows, 1 for linux/MacOS).
     *
     * @return The IOCTL CCID "Escape" command identifier adapted to the OS.
     * @since 2.1.0
     */
    virtual int getIoctlCcidEscapeCommandId() const = 0;

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

/**
 *
 */
inline std::string operator+(std::string str, const PcscReader::DisconnectionMode dm)
{
    switch (dm) {
    case PcscReader::DisconnectionMode::RESET:
        return str + "RESET";
    case PcscReader::DisconnectionMode::LEAVE:
        return str + "LEAVE";
    case PcscReader::DisconnectionMode::UNPOWER:
        return str + "UNPOWER";
    case PcscReader::DisconnectionMode::EJECT:
        return str + "EJECT";
    default:
        return str + "UNKNOWN";
    }
}

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */