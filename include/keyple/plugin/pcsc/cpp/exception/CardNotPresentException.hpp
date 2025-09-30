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

#include "keyple/plugin/pcsc/cpp/exception/CardException.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {
namespace exception {


class CardNotPresentException : public CardException {
public:
    /**
     *
     */
    explicit CardNotPresentException(const std::string& msg)
    : CardException(msg) {}

    /**
     *
     */
    CardNotPresentException(
        const std::string& msg, const std::shared_ptr<Exception> cause)
    : CardException(msg, cause) {}
};

}
}
}
}
}
