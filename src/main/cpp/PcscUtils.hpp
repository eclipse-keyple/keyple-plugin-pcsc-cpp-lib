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
 * does not. This function formats the error code as a hexadecimal string.
 */
inline std::string
pcsc_stringify_error(uint64_t rv)
{
    static char out[20];
    sprintf_s(out, sizeof(out), "0x%08X", static_cast<unsigned int>(rv));
    return std::string(out);
}

} // namespace cpp
} // namespace pcsc
} // namespace plugin
} // namespace keyple

#endif // defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
