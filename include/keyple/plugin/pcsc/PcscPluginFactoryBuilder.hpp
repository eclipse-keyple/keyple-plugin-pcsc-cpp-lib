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

#include <map>
#include <memory>
#include <string>

#include "keyple/core/util/cpp/Pattern.hpp"
#include "keyple/plugin/pcsc/KeyplePluginPcscExport.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactory.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::util::cpp::Pattern;

/**
 * Builds instances of PcscPluginFactory from values configured by the setters.
 *
 * <p>Note: all setters of this class are optional.<br>
 * It is possible to assign later a protocol type at the reader level using the
 * method PcscReader#setContactless(bool). <br>
 * A set of default protocol identification rules is also proposed.
 *
 * @see PcscCardCommunicationProtocol
 * @since 2.0.0
 */
class KEYPLEPLUGINPCSC_API PcscPluginFactoryBuilder final {
public:

    /**
     * Builder to build a PcscPluginFactory.
     *
     * @since 2.0.0
     */
    class KEYPLEPLUGINPCSC_API Builder {
    public:
        friend PcscPluginFactoryBuilder;

        /**
         * Sets a filter based on regular expressions to make the plugin able to
         * identify a contact reader from its name.
         *
         * <p>Readers whose names match the provided regular expression will be
         * considered contact type readers.
         *
         * <p>For example, the string ".*less.*" could identify all readers
         * having "less" in their name as contactless readers.
         *
         * <p>Names are not always as explicit, so it is sometimes better to
         * test the brand and model.
         * <br>
         * Commonly used contact readers include "Cherry TC" or "Identive".<br>
         * Thus, an application using these readers should call this method with
         * {@code ".*(Cherry TC|Identive).*"} as an argument.
         *
         * @param contactReaderIdentificationFilter A string a regular
         *        expression.
         * @return This builder.
         * @since 2.0.0
         * @deprecated Useless method that will be removed soon, see
         *             #useContactlessReaderIdentificationFilter(String)
         */
        Builder& useContactReaderIdentificationFilter(
            const std::string& contactReaderIdentificationFilter);

        /**
         * Overwrites the default filter with the provided filter based on
         * regular expressions to make the plugin able to identify a contact
         * reader from its name.
         *
         * <p>Readers whose names match the provided regular expression will be
         * considered contactless type readers.
         *
         * <p>The default value is {@value #DEFAULT_CONTACTLESS_READER_FILTER}
         *
         * @param contactlessReaderIdentificationFilter A regular expression.
         * @return This builder.
         * @throw IllegalArgumentException If the provided string is null, empty
         *        or invalid.
         * @see #useContactReaderIdentificationFilter(String)
         * @since 2.0.0
         */
        Builder& useContactlessReaderIdentificationFilter(
            const std::string& contactlessReaderIdentificationFilter);

        /**
         * Updates a protocol identification rule.
         *
         * <p>A protocol rule is a regular expression contained in a String.
         *
         * <ul>
         *   <li>If a rule already exists for the provided protocol, it is
         *   replaced.
         *   <li>If no rule exists for the provided protocol, it is added.
         *   <li>If the rule is null, the protocol is disabled.
         * </ul>
         *
         * @param readerProtocolName A not empty String.
         * @param protocolRule null to disable the protocol.
         * @return This builder.
         * @throw IllegalArgumentException If one of the argument is null or
         *        empty
         * @since 2.0.0
         */
        Builder& updateProtocolIdentificationRule(
            const std::string& readerProtocolName,
            const std::string& protocolRule);

        /**
         * Sets the cycle duration for card monitoring (insertion and removal).
         *
         * <p>This parameter allows you to reduce the default value of 500 ms if
         * the underlying layer doesn't allow multiple accesses to the reader,
         * resulting in high latency in processing.
         *
         * <p>This is particularly useful under Linux when, in observed mode,
         * the APDU processing is not executed in the same thread as the one
         * used for card detection. A similar case occurs when several readers
         * are observed simultaneously.
         *
         * <p>The value given here should be determined with care, as it can
         * lead to a significant increase in the CPU load generated by the
         * application.
         *
         * @param cycleDuration The cycle duration for card monitoring, a
         *        positive integer in milliseconds.
         * @return This builder.
         * @throw IllegalArgumentException If the provided value is out of
         *        range.
         * @since 2.3.0
         */
        Builder& setCardMonitoringCycleDuration(const int cycleDuration);

        /**
         * Replace the default jnasmartcardio provider by the provider given in
         * argument.
         *
         * @param provider The provider to use, must not be null.
         * @return This builder.
         * @throws IllegalArgumentException If the argument is null.
         * @since 2.4.0
         */
        //Builder& setProvider(Provider provider);

        /**
         * Returns an instance of PcscPluginFactory created from the fields set
         * on this builder.
         *
         * <p>The type of reader is determined using a regular expression
         * applied to its name. <br>
         * The default regular expression is
         * {@value DEFAULT_CONTACTLESS_READER_FILTER}.<br>
         * Readers that do not match this regular expression are considered
         * contact type. <br>
         * It is possible to redefine the contactless reader filter via the
         * method #useContactlessReaderIdentificationFilter(String).
         *
         * @return A PcscPluginFactory.
         * @since 2.0.0
         */
        std::shared_ptr<PcscPluginFactory> build();

    private:
        /**
         *
         */
        static const std::string DEFAULT_CONTACTLESS_READER_FILTER;

        /**
         *
         */
        std::shared_ptr<Pattern> mContactlessReaderIdentificationFilterPattern;

        /**
         *
         */
        std::map<std::string, std::string> mProtocolRulesMap;

        /**
         *
         */
        int mCardMonitoringCycleDuration;

        /**
         * (private)<br>
         *
         * Constructs an empty Builder. The default value of all strings is
         * null, the default value of the map is an empty map.
         */
        Builder();
    };

    /**
     * Creates builder to build a {@link PcscPluginFactory}.
     *
     * @return created builder
     * @since 2.0.0
     */
    static std::unique_ptr<Builder> builder();

private:
    /**
     *
     */
    PcscPluginFactoryBuilder() {}
};

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
