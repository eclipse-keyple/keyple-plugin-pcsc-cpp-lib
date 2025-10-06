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

#include "keyple/plugin/pcsc/cpp/TerminalFactory.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include "PcscUtils.hpp"

#include "keyple/plugin/pcsc/cpp/exception/CardTerminalException.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {

using keyple::plugin::pcsc::cpp::exception::CardTerminalException;

std::shared_ptr<TerminalFactory> TerminalFactory::mInstance;

std::shared_ptr<TerminalFactory>
TerminalFactory::getDefault()
{
    if (mInstance == nullptr) {
        mInstance = std::shared_ptr<TerminalFactory>(new TerminalFactory());

    }

    return mInstance;
}

const std::vector<std::string>
TerminalFactory::listTerminals()
{
    ULONG ret;
    char* readers = NULL;
    char* ptr = NULL;
    DWORD len = 0;
    static std::vector<std::string> list;

    /* Clear list */
    list.clear();

    ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &mContext);
    if (ret != SCARD_S_SUCCESS) {
        throw CardTerminalException(pcsc_stringify_error(ret));
    }

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
        list.push_back(s);
        ptr += strlen(ptr) + 1;
    }

    SCardReleaseContext(mContext);
    free(readers);

    return list;
}

std::shared_ptr<CardTerminals>
TerminalFactory::terminals()
{
    LONG ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &mContext);
    if (ret != SCARD_S_SUCCESS) {
        throw CardTerminalException(pcsc_stringify_error(ret));
    }

    return std::make_shared<CardTerminals>(mContext);
}

} /* namespace cpp */
} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */