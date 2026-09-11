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

#include <string>
#include <cstdint>
#include <cstdio>

#include <winscard.h>

// SCARD_PROTOCOL_ANY is not defined in Windows winscard.h
#ifndef SCARD_PROTOCOL_ANY
#define SCARD_PROTOCOL_ANY (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1)
#endif

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

/**
 * Windows-specific utility to stringify PC/SC error codes.
 * On Linux, pcsc-lite provides pcsc_stringify_error(), but Windows winscard.h
 * does not.
 *
 * The symbolic name matters: PcscPluginAdapter dispatches on it to decide
 * whether an enumeration failure is recoverable, exactly as the Java reference
 * does, where the PCSCException message IS the SCARD_* name. Returning only a
 * hexadecimal code made every one of those tests fail, so the recovery path
 * that re-establishes the PC/SC context was unreachable. The code is appended
 * so nothing is lost, and unknown values still come out as hexadecimal.
 */
inline std::string
pcsc_stringify_error(uint64_t rv)
{
    const char* name;

    switch (static_cast<LONG>(rv)) {
    case SCARD_S_SUCCESS: name = "SCARD_S_SUCCESS"; break;
    case SCARD_E_CANCELLED: name = "SCARD_E_CANCELLED"; break;
    case SCARD_E_INVALID_HANDLE: name = "SCARD_E_INVALID_HANDLE"; break;
    case SCARD_E_INVALID_PARAMETER: name = "SCARD_E_INVALID_PARAMETER"; break;
    case SCARD_E_NO_MEMORY: name = "SCARD_E_NO_MEMORY"; break;
    case SCARD_E_TIMEOUT: name = "SCARD_E_TIMEOUT"; break;
    case SCARD_E_SHARING_VIOLATION: name = "SCARD_E_SHARING_VIOLATION"; break;
    case SCARD_E_NO_SMARTCARD: name = "SCARD_E_NO_SMARTCARD"; break;
    case SCARD_E_PROTO_MISMATCH: name = "SCARD_E_PROTO_MISMATCH"; break;
    case SCARD_E_NOT_READY: name = "SCARD_E_NOT_READY"; break;
    case SCARD_E_READER_UNAVAILABLE: name = "SCARD_E_READER_UNAVAILABLE"; break;
    case SCARD_E_NO_SERVICE: name = "SCARD_E_NO_SERVICE"; break;
    case SCARD_E_SERVICE_STOPPED: name = "SCARD_E_SERVICE_STOPPED"; break;
    case SCARD_E_NO_READERS_AVAILABLE:
        name = "SCARD_E_NO_READERS_AVAILABLE";
        break;
    case SCARD_E_NOT_TRANSACTED: name = "SCARD_E_NOT_TRANSACTED"; break;
    case SCARD_E_COMM_DATA_LOST: name = "SCARD_E_COMM_DATA_LOST"; break;
    case SCARD_E_UNKNOWN_READER: name = "SCARD_E_UNKNOWN_READER"; break;
    case SCARD_F_COMM_ERROR: name = "SCARD_F_COMM_ERROR"; break;
    case SCARD_F_INTERNAL_ERROR: name = "SCARD_F_INTERNAL_ERROR"; break;
    case SCARD_F_UNKNOWN_ERROR: name = "SCARD_F_UNKNOWN_ERROR"; break;
    case SCARD_W_UNPOWERED_CARD: name = "SCARD_W_UNPOWERED_CARD"; break;
    case SCARD_W_UNRESPONSIVE_CARD: name = "SCARD_W_UNRESPONSIVE_CARD"; break;
    case SCARD_W_RESET_CARD: name = "SCARD_W_RESET_CARD"; break;
    case SCARD_W_REMOVED_CARD: name = "SCARD_W_REMOVED_CARD"; break;
    default: name = nullptr; break;
    }

    char code[12];
    sprintf_s(code, sizeof(code), "0x%08X", static_cast<unsigned int>(rv));

    return name != nullptr ? std::string(name) + " (" + code + ")"
                           : std::string(code);
}

/**
 * Checks if a PC/SC return value corresponds to a card communication loss or
 * card removal rather than a reader hardware breakdown.
 */
inline bool
isCardCommunicationError(uint64_t rv)
{
    return rv == static_cast<uint64_t>(SCARD_W_REMOVED_CARD)
        || rv == static_cast<uint64_t>(SCARD_W_RESET_CARD)
        || rv == static_cast<uint64_t>(SCARD_W_UNPOWERED_CARD)
        || rv == static_cast<uint64_t>(SCARD_W_UNRESPONSIVE_CARD)
        || rv == static_cast<uint64_t>(SCARD_E_NO_SMARTCARD)
        || rv == static_cast<uint64_t>(SCARD_E_NOT_TRANSACTED)
        || rv == static_cast<uint64_t>(SCARD_E_COMM_DATA_LOST)
        /* SCARD_F_INTERNAL_ERROR, commonly returned on a contactless
         * card tear-off. */
        || rv == 0x80100001;
}

} // namespace cpp
} // namespace pcsc
} // namespace plugin
} // namespace keyple

#endif // defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
