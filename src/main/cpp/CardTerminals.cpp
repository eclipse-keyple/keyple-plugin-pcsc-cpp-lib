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

#include "keyple/plugin/pcsc/cpp/CardTerminals.hpp"

#include <cstdint>
#include <cstring>
#include <string>

#include "PcscUtils.hpp"

#include "keyple/core/util/cpp/exception/IllegalArgumentException.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardException.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardTerminalException.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using keyple::core::util::cpp::exception::IllegalArgumentException;
using keyple::plugin::pcsc::cpp::exception::CardException;
using keyple::plugin::pcsc::cpp::exception::CardTerminalException;

CardTerminals::CardTerminals(SCARDCONTEXT& context)
: mContext(context)
{
}

void
CardTerminals::waitForChange()
{
    waitForChange(0);
}

bool
CardTerminals::waitForChange(long timeout)
{
    if (timeout < 0) {
        throw IllegalArgumentException(
            "Negative timeout " + std::to_string(timeout));
    } else if (timeout == 0) {
        timeout = INFINITE;
    }

    mZombieReaders.clear();

    for (auto& reader: mKnownReaders) {
        reader.dwCurrentState = reader.dwEventState;
        reader.dwEventState = 0;
    }

    LONG rv = SCardGetStatusChange(
        mContext, timeout, mKnownReaders.data(), static_cast<DWORD>(mKnownReaders.size()));
    if (rv == static_cast<LONG>(SCARD_E_TIMEOUT)) {
        return false;
    }

    return true;
}

std::shared_ptr<CardTerminal>
CardTerminals::getTerminal(const std::string& name)
{
    try {
        for (const auto& terminal : list()) {
            if (terminal->getName() == name) {
                return terminal;
            }
        }

        return nullptr;

    } catch (const CardException&) {
        return nullptr;
    }
}

const std::vector<std::shared_ptr<CardTerminal>>
CardTerminals::list()
{
    return list(State::ALL);
}


const std::vector<std::shared_ptr<CardTerminal>>
CardTerminals::list(const State /*state*/)
{
    ULONG ret;
    char* readers = NULL;
    char* ptr = NULL;
    DWORD len = 0;
    static std::vector<std::shared_ptr<CardTerminal>> list;

    /* Clear list */
    list.clear();

    ret = SCardListReaders(mContext, NULL, NULL, &len);
    if (ret != SCARD_S_SUCCESS) {
        throw CardTerminalException(pcsc_stringify_error(ret));
    }

    readers = static_cast<char*>(calloc(len, sizeof(char)));

    if (readers == NULL) {
        /* No readers to add to list */
        return list;
    }

    ret = SCardListReaders(mContext, NULL, readers, &len);
    if (ret != SCARD_S_SUCCESS) {
        throw CardTerminalException(pcsc_stringify_error(ret));
    }

    ptr = readers;

    if (!ptr) {
        return list;
    }

    while (*ptr) {
        std::string s(ptr);
        auto terminal = std::make_shared<CardTerminal>(shared_from_this(), s);
        list.push_back(terminal);
        ptr += strlen(ptr) + 1;
    }

    free(readers);

    return list;
}

} /* namespace cpp */
} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */