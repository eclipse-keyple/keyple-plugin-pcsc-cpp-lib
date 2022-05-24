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

/* Keyple Core Util */
#include "Exception.h"

namespace keyple {
namespace plugin {
namespace pcsc {
namespace cpp {
namespace exception {

using namespace keyple::core::util::cpp::exception;

class CardException : public Exception {
public:
    /**
     *
     */
    explicit CardException(const std::string& msg) : Exception(msg) {}

    /**
     *
     */
    CardException(const std::string& msg, const std::shared_ptr<Exception> cause)
    : Exception(msg, cause) {}
};

}
}
}
}
}
