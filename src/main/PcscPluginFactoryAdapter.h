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

#include <map>
#include <string>

/* Keyple Plugin Pcsc */
#include "PcscPluginFactory.h"

/* Keyple Core Plugin */
#include "PluginFactorySpi.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::plugin::spi;

/**
 * (package-private)<br>
 * Factory of {@link PcscPlugin}.
 *
 * @since 2.0.0
 */
class PcscPluginFactoryAdapter final : public PcscPluginFactory, public PluginFactorySpi {
public:
    /**
     * (package-private)<br>
     * The plugin name
     *
     * @since 2.0.0
     */
    static const std::string PLUGIN_NAME;

    /**
     * (package-private)<br>
     * Creates an instance, sets the fields from the factory builder.
     *
     * @since 2.0.0
     */
    PcscPluginFactoryAdapter(const std::string& contactReaderIdentificationFilter,
                             const std::string& contactlessReaderIdentificationFilter,
                             const std::map<std::string, std::string>& protocolRulesMap);

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string& getPluginApiVersion() const override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string& getCommonApiVersion() const override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string& getPluginName() const override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    std::shared_ptr<PluginSpi> getPlugin() override;

private:
    /**
     * 
     */
    const std::string mContactReaderIdentificationFilter;

    /**
     * 
     */
    const std::string mContactlessReaderIdentificationFilter;

    /**
     * 
     */
    const std::map<std::string, std::string> mProtocolRulesMap;
};

}
}
}
