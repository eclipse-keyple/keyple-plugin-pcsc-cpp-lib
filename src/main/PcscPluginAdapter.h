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

/* Keyple Plugin Pcsc */
#include "AbstractPcscPluginAdapter.h"
#include "AbstractPcscReaderAdapter.h"
#include "CardTerminal.h"

namespace keyple {
namespace plugin {
namespace pcsc {

/**
 * (package-private)<br>
 * Implementation of {@link AbstractPcscPluginAdapter} suitable for platforms other than Windows.
 *
 * <p>Provides a createInstance method that receives a boolean as an argument to indicate that the
 * platform is MacOS. <br>
 * This information is used to create readers capable of handling the technical issues specific to
 * this platform.
 *
 * @since 2.0.0
 */
class PcscPluginAdapter
: public AbstractPcscPluginAdapter, public std::enable_shared_from_this<PcscPluginAdapter> {
public:
    /**
     * Gets the single instance of PcscPluginAdapter.
     *
     * @return Single instance of PcscPluginAdapter
     * @since 2.0.0
     */
    static std::shared_ptr<PcscPluginAdapter> getInstance();

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::vector<std::shared_ptr<CardTerminal>> getCardTerminals() const override;

    /**
     * {@inheritDoc}
     *
     * <p>Creates a specific instance if the platform is MacOS, a standard one for others platforms
     * (e.g. Linux).
     * 
     * C++: covariance doesn't work on shared_ptr, this function should (and does) return a
     *      std::shared_ptr<AbstractPcscReaderAdapter>
     * 
     * @param terminal The smartcard.io {@link CardTerminal}.
     * @return A not null reference.
     * @since 2.0.0
     */
    std::shared_ptr<ReaderSpi> createReader(std::shared_ptr<CardTerminal> terminal) override;

    /**
     * (private)<br>
     * Constructor.
     * 
     * C++: cannot be private if used by the getInstance() function...
     */
    PcscPluginAdapter();

private:
    /**
     * The 'volatile' qualifier ensures that read access to the object will only be allowed once the
     * object has been fully initialized. <br>
     * This qualifier is required for 'lazy-singleton' pattern with double-check method, to be
     * thread-safe.
     */
    static std::shared_ptr<PcscPluginAdapter> INSTANCE;
};

}
}
}
