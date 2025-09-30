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

#include <string>

/* Keyple Plugin Pcsc */
#include "keyple/plugin/pcsc/KeyplePluginPcscExport.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

/**
 * Non-exhaustive list of protocols supported by common contact PC/SC readers.
 *
 * <p>TODO Improve protocol identification
 *
 * @since 2.0.0
 * @deprecated
 */
class KEYPLEPLUGINPCSC_API PcscSupportedContactProtocol {
public:
    /**
     * ISO7816-3 Card (unspecified communication protocol)<br>
     * Default rule = <b>{@code 3.*}</b>
     *
     * @since 2.0.0
     */
    static const PcscSupportedContactProtocol ISO_7816_3;

    /**
     * ISO7816-3 Card communicating with T=0 protocol<br>
     * Default rule = <b>{@code 3.*}</b>
     *
     * @since 2.0.0
     */
    static const PcscSupportedContactProtocol ISO_7816_3_T0;

    /**
     * ISO7816-3 Card communicating with T=1 protocol<br>
     * Default rule = <b>{@code 3.*}</b>
     *
     * @since 2.0.0
     */
    static const PcscSupportedContactProtocol ISO_7816_3_T1;

    /**
     * (private-package)<br>
     * Gets the default associated to the protocol.
     *
     * @return A byte
     * @since 2.0.0
     */
    const std::string& getDefaultRule() const;

    /**
     *
     */
    const std::string& getName() const;

private:
    /**
     *
     */
    const std::string mName;

    /**
     *
     */
    const std::string mDefaultRule;

    /**
     * Constructor
     *
     * @param defaultRule The default rule.
     * @since 2.0.0
     */
    PcscSupportedContactProtocol(
        const std::string& name, const std::string& defaultRule);
};

}
}
}
