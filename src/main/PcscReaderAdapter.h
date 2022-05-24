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

#include <atomic>
#include <memory>
#include <typeinfo>

/* Keyple Plugin Pcsc */
#include "AbstractPcscReaderAdapter.h"

/* Keyple Core Plugin */
#include "WaitForCardInsertionBlockingSpi.h"

/* Keyple Core Util */
#include "LoggerFactory.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::plugin::spi::reader::observable::state::insertion;
using namespace keyple::core::util::cpp;

/**
 * (package-private)<br>
 * Implementation of AbstractPcscReaderAdapter suitable for platforms other than MacOS.
 *
 * @since 2.0.0
 */
class PcscReaderAdapter final
: public AbstractPcscReaderAdapter, public WaitForCardInsertionBlockingSpi {
public:
    /**
     * (package-private)<br>
     *
     * @since 2.0.0
     */
    PcscReaderAdapter(std::shared_ptr<CardTerminal> terminal, 
                      std::shared_ptr<AbstractPcscPluginAdapter> pluginAdapter);
    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void waitForCardInsertion() override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void stopWaitForCardInsertion() override;

private:
    /**
     * 
     */
    const std::unique_ptr<Logger> mLogger = LoggerFactory::getLogger(typeid(PcscReaderAdapter));

    /*
     * The latency delay value (in ms) determines the maximum time during which the
     * waitForCardPresent blocking functions will execute.
     * This will correspond to the capacity to react to the interrupt signal of the thread.
     */
    static const long INSERTION_LATENCY;

    /**
     * 
     */
    std::atomic<bool> mLoopWaitCard;
};

}
}
}
