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

#include "PcscReaderAdapter.h"

/* Keyple Plugin Pcsc */
#include "CardException.h"

/* Keyple Core Plugin */
#include "ReaderIOException.h"
#include "TaskCanceledException.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::plugin;
using namespace keyple::plugin::pcsc::cpp::exception;

const long PcscReaderAdapter::INSERTION_LATENCY = 500;

PcscReaderAdapter::PcscReaderAdapter(std::shared_ptr<CardTerminal> terminal,
                                     std::shared_ptr<AbstractPcscPluginAdapter> pluginAdapter)
: AbstractPcscReaderAdapter(terminal, pluginAdapter),
  mLoopWaitCard(false) {}

void PcscReaderAdapter::waitForCardInsertion()
{
    mLogger->trace("%: start waiting for the insertion of a card in a loop with a latency of %" \
                   " ms\n",
                   getName(),
                   INSERTION_LATENCY);


    /* Activate loop */
    mLoopWaitCard = true;

    try {
        while (mLoopWaitCard) {
            if (getTerminal()->waitForCardPresent(INSERTION_LATENCY)) {
                /* Card inserted */
                mLogger->trace("%: card inserted\n", getName());
                return;
            }

            // if (Thread.interrupted()) {
            //     break;
            // }
        }
    } catch (const CardException& e) {
        /* Here, it is a communication failure with the reader */
        throw ReaderIOException(getName() +
                                ": an error occurred while waiting for a card insertion.",
                                std::make_shared<CardException>(e));
    }

    throw TaskCanceledException(getName() +
                                ": the wait for a card insertion task has been cancelled.");
}

void PcscReaderAdapter::stopWaitForCardInsertion()
{
    mLogger->trace("%: stop waiting for card insertion requested\n", getName());

    mLoopWaitCard = false;
}

}
}
}
