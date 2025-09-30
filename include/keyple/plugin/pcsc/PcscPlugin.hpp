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

#include "keyple/core/common/KeyplePluginExtension.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::common::KeyplePluginExtension;

/**
 * PC/SC specific KeyplePluginExtension.
 *
 * @since 2.0.0
 */
class PcscPlugin : public KeyplePluginExtension {
public:
    /**
     * Visrtual destructor.
     */
    virtual ~PcscPlugin() = default;
};

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
