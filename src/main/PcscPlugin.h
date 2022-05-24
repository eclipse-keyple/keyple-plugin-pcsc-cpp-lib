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

/* Keyple Core Common */
#include "KeyplePluginExtension.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::common;

/**
 * PC/SC specific {@link KeyplePluginExtension}.
 *
 * @since 2.0.0
 */
class PcscPlugin : public KeyplePluginExtension {
public:
    /**
     * 
     */
    virtual ~PcscPlugin() = default;    
};

}
}
}
