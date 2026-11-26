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

#include <iostream>

#include "keyple/plugin/pcsc/PcscPluginFactory.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"

int
main()
{
    /* Calls into the actual compiled library (not just a header), proving
     * the binary resolved and linked correctly through Conan/CMake -- this
     * only builds the factory config, it does not open a PC/SC session. */
    const std::shared_ptr<keyple::plugin::pcsc::PcscPluginFactory> factory
        = keyple::plugin::pcsc::PcscPluginFactoryBuilder::builder()->build();

    std::cout << "Keyple::Plugin::Pcsc package resolved and linked correctly ("
              << (factory != nullptr ? "factory built" : "factory build failed") << ")" << std::endl;
    return factory != nullptr ? 0 : 1;
}
