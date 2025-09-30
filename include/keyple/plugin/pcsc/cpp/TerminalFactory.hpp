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

#include "keyple/plugin/pcsc/cpp/CardTerminals.hpp"

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
 * A factory for CardTerminal objects.
 */
class TerminalFactory {
public:
    /**
     * Singletons should not be cloneable.
     */
    TerminalFactory(TerminalFactory& other) = delete;

    /**
     *
     */
    virtual ~TerminalFactory() = default;

    /**
     * Singletons should not be assignable.
     */
    void operator=(const TerminalFactory&) = delete;

    /**
     *
     */
    static std::shared_ptr<TerminalFactory> getDefault();

    /**
     * Returns a new CardTerminals object encapsulating the terminals
     * supported by this factory.
     *
     * @return a new CardTerminals object encapsulating the terminals
     * supported by this factory.
     */
    std::shared_ptr<CardTerminals> terminals();

private:
    /**
     *
     */
    SCARDCONTEXT mContext;

    /**
     *
     */
    static std::shared_ptr<TerminalFactory> mInstance;

    /**
     *
     */
    TerminalFactory() = default;

    /**
     *
     */
    const std::vector<std::string>
    listTerminals();
};

} /* namespace cpp */
} /* namespace pcsc*/
} /* namespace pcsc */
} /* namespace plugin */