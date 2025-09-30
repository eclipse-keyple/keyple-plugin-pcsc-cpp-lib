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
#include <vector>

#include "keyple/core/plugin/spi/ObservablePluginSpi.hpp"
#include "keyple/core/plugin/spi/reader/ReaderSpi.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/Pattern.hpp"
#include "keyple/plugin/pcsc/PcscPlugin.hpp"
#include "keyple/plugin/pcsc/PcscReaderAdapter.hpp"
#include "keyple/plugin/pcsc/cpp/CardTerminal.hpp"
#include "keyple/plugin/pcsc/cpp/CardTerminals.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::plugin::spi::ObservablePluginSpi;
using keyple::core::plugin::spi::reader::ReaderSpi;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::Pattern;
using keyple::plugin::pcsc::cpp::CardTerminal;
using keyple::plugin::pcsc::cpp::CardTerminals;

class PcscReaderAdapter;

/**
 * Implementation of PcscPlugin.
 *
 * @since 2.0.0
 */
class PcscPluginAdapter final
: public PcscPlugin,
  public ObservablePluginSpi,
  public std::enable_shared_from_this<PcscPluginAdapter> {
public:
    /**
     * Gets the single instance.
     *
     * @return This instance.
     * @since 2.0.0
     */
    static std::shared_ptr<PcscPluginAdapter> getInstance();

    /**
     * Creates a new instance of ReaderSpi from a CardTerminal.
     *
     * <p>Note: this method is platform dependent.
     *
     * @param terminal A CardTerminal.
     * @return A not null reference.
     * @since 2.0.0
     */
    std::unique_ptr<PcscReaderAdapter> createReader(
        std::shared_ptr<CardTerminal> terminal);

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    int getMonitoringCycleDuration() const override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::vector<std::string> searchAvailableReaderNames() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string& getName() const override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::vector<std::shared_ptr<ReaderSpi>> searchAvailableReaders() override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void onUnregister() override;

    /**
     * (private) Gets the list of terminals.
     *
     * <p>The aim is to handle the exception possibly raised by the underlying
     * smartcard.io method.
     *
     * @return An empty list if no reader is available.
     * @throws PluginIOException If an error occurs while accessing the list.
     */
    const std::vector<std::shared_ptr<CardTerminal>> getCardTerminalList();


    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    std::shared_ptr<ReaderSpi> searchReader(const std::string& readerName) override;

    /**
     * Gets the protocol rule associated to the provided protocol.
     *
     * <p>The protocol rule is a regular expression to be applied on the ATR.
     *
     * @param readerProtocol The reader protocol.
     * @return Null if no protocol rules defined for the provided protocol.
     * @since 2.0.0
     */
    virtual const std::string& getProtocolRule(
        const std::string& readerProtocol) const final;

    /**
     * Attempts to determine the transmission mode of the reader whose name is
     * provided.<br>
     * This determination is made by a test based on the regular expressions.
     *
     * @param readerName A string containing the reader name
     * @return True if the reader is contactless, false if not.
     * @since 2.0.0
     */
    bool isContactless(const std::string& readerName);

    /**
     * Sets the filter to identify contactless readers.
     *
     * @param contactlessReaderIdentificationFilter A regular expression pattern.
     *        set.
     * @return The object instance.
     * @since 2.0.0
     */
    PcscPluginAdapter& setContactlessReaderIdentificationFilterPattern(
        const std::shared_ptr<Pattern> contactlessReaderIdentificationFilter);

    /**
     * Adds a map of rules to the current default map.
     *
     * <p>Already existing items are overridden, new items are added.
     *
     * @param protocolRulesMap The regex based filter.
     * @return The object instance.
     * @since 2.0.0
     */
    PcscPluginAdapter& addProtocolRulesMap(
        const std::map<std::string, std::string> protocolRulesMap);

    /**
     * Sets the cycle duration for card presence/absence monitoring.
     *
     * @param cardMonitoringCycleDuration The duration of the card monitoring
     *        cycle in milliseconds.
     * @return The object instance.
     * @since 2.3.0
     */
    PcscPluginAdapter& setCardMonitoringCycleDuration(
        const int cardMonitoringCycleDuration);

    /**
     * Constructor.
     *
     * C++: cannot be private if used by the getInstance() function...
     */
    PcscPluginAdapter();

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger =
        LoggerFactory::getLogger(typeid(PcscPluginAdapter));

    /**
     * Singleton instance of the class
     *
     * <p>'volatile' qualifier ensures that read access to the object will only
     * be allowed once the object has been fully initialized. <br>
     * This qualifier is required for "lazy-singleton" pattern with double-check
     * method, to be thread-safe.
     */
    static std::shared_ptr<PcscPluginAdapter> INSTANCE;

    /**
     *
     */
    static const int MONITORING_CYCLE_DURATION_MS;

    /**
     *
     */
    std::map<std::string, std::string> mProtocolRulesMap;

    /**
     *
     */
    std::shared_ptr<CardTerminals> mTerminals;

    /**
     *
     */
    bool mIsCardTerminalsInitialized;

    /**
     *
     */
    std::shared_ptr<Pattern> mContactlessReaderIdentificationFilterPattern;

    /**
     *
     */
    int mCardMonitoringCycleDuration;
};

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
