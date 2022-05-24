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

#include "PcscPluginAdapter.h"

/* Keyple Plugin Pcsc */
#include "CardTerminal.h"
#include "PcscPluginFactoryAdapter.h"
#include "PcscReaderAdapter.h"

namespace keyple {
namespace plugin {
namespace pcsc {

std::shared_ptr<PcscPluginAdapter> PcscPluginAdapter::INSTANCE;

PcscPluginAdapter::PcscPluginAdapter()
: AbstractPcscPluginAdapter(PcscPluginFactoryAdapter::PLUGIN_NAME) {}

std::shared_ptr<PcscPluginAdapter> PcscPluginAdapter::getInstance()
{
    if  (!INSTANCE) {
        INSTANCE = std::make_shared<PcscPluginAdapter>();
    }

    return INSTANCE;
}

const std::vector<std::shared_ptr<CardTerminal>> PcscPluginAdapter::getCardTerminals() const
{
    std::vector<std::shared_ptr<CardTerminal>> terminals;

    const std::vector<std::string>& terminalNames = CardTerminal::listTerminals();
    for (const auto& terminalName : terminalNames) {
        terminals.push_back(std::make_shared<CardTerminal>(terminalName));
    }

    return terminals;
}

std::shared_ptr<ReaderSpi> PcscPluginAdapter::createReader(std::shared_ptr<CardTerminal> terminal)
{
    return std::dynamic_pointer_cast<ReaderSpi>(
               std::make_shared<PcscReaderAdapter>(terminal, shared_from_this()));
}

}
}
}
