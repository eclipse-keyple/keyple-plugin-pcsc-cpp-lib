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
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

/* Keyple Core Util */
#include "LoggerFactory.h"

/* Keyple Core Plugin */
#include "ObservablePluginSpi.h"

/* Keyple Plugin Pcsc */
#include "PcscPlugin.h"

#include "CardTerminal.h"


namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::plugin::spi;
using namespace keyple::core::util::cpp;
using namespace keyple::plugin::pcsc::cpp;

/**
 * (package-private) <br>
 * Abstract class for all PC/SC plugin adapters.
 *
 * @since 2.0.0
 */
class AbstractPcscPluginAdapter : public PcscPlugin, public ObservablePluginSpi {
public:
    /**
     * (package-private)<br>
     * Common constructor for all Pcsc plugin adapters instances.
     *
     * @param name The name of the plugin.
     * @since 2.0.0
     */
    AbstractPcscPluginAdapter(const std::string& name);

    /**
     * 
     */
    virtual ~AbstractPcscPluginAdapter() = default;

    /**
     * (package-private)<br>
     * Sets the filter to identify contact readers.
     *
     * @param contactReaderIdentificationFilter A regular expression, null if the filter is not set.
     * @return The object instance.
     * @since 2.0.0
     */
    virtual AbstractPcscPluginAdapter& setContactReaderIdentificationFilter(
        const std::string& contactReaderIdentificationFilter) final;

    /**
     * (package-private)<br>
     * Sets the filter to identify contactless readers.
     *
     * @param contactlessReaderIdentificationFilter A regular expression, null if the filter is not
     *     set.
     * @return The object instance.
     * @since 2.0.0
     */
    virtual AbstractPcscPluginAdapter& setContactlessReaderIdentificationFilter(
        const std::string& contactlessReaderIdentificationFilter) final;

    /**
     * (package-private)<br>
     * Adds a map of rules to the current default map.
     *
     * <p>Already existing items are overridden, new items are added.
     *
     * @param protocolRulesMap The regex based filter.
     * @return The object instance.
     * @since 2.0.0
     */
    virtual AbstractPcscPluginAdapter& addProtocolRulesMap(
        const std::map<std::string, std::string> protocolRulesMap) final;

    /**
     * (package-private)<br>
     * Gets the protocol rule associated to the provided protocol.
     *
     * <p>The protocol rule is a regular expression to be applied on the ATR.
     *
     * @param readerProtocol The reader protocol.
     * @return Null if no protocol rules defined for the provided protocol.
     * @since 2.0.0
     */
    virtual const std::string& getProtocolRule(const std::string& readerProtocol) const final;

    /**
     * (package-private)<br>
     * Creates a new instance of {@link ReaderSpi} from a {@link CardTerminal}.
     *
     * <p>Note: this method is platform dependent.
     *
     * @param terminal A smartcard.io {@link CardTerminal}.
     * @return A not null reference.
     * @since 2.0.0
     */
    virtual std::shared_ptr<ReaderSpi> createReader(std::shared_ptr<CardTerminal> terminal) = 0;

    /**
     * (package-private)<br>
     * Gets a new {@link CardTerminals} object encapsulating the available terminals.
     *
     * <p>Note: this method is platform dependent.
     *
     * @return A {@link CardTerminals} reference
     * @since 2.0.0
     */
    virtual const std::vector<std::shared_ptr<CardTerminal>> getCardTerminals() const = 0;

    /**
     * (package-private)<br>
     * Attempts to determine the transmission mode of the reader whose name is provided.<br>
     * This determination is made by a test based on the regular expressions.
     *
     * @param readerName A string containing the reader name
     * @return True if the reader is contactless, false if not.
     * @throws IllegalStateException If the mode of transmission could not be determined
     * @throws PatternSyntaxException If the expression's syntax is invalid
     * @since 2.0.0
     */
    virtual bool isContactless(const std::string& readerName) final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    virtual const std::string& getName() const override final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    virtual const std::vector<std::shared_ptr<ReaderSpi>> searchAvailableReaders() override final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    virtual void onUnregister() override final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    virtual int getMonitoringCycleDuration() const override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    virtual const std::vector<std::string> searchAvailableReaderNames() override final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    virtual std::shared_ptr<ReaderSpi> searchReader(const std::string& readerName) override final;

private:
    /**
     * 
     */
    const std::unique_ptr<Logger> mLogger = 
        LoggerFactory::getLogger(typeid(AbstractPcscPluginAdapter));

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
    const std::string mName;
    
    /**
     * 
     */
    std::string mContactReaderIdentificationFilter;
    
    /**
     * 
     */
    std::string mContactlessReaderIdentificationFilter;

    /**
     * (private) Gets the list of terminals provided by smartcard.io.
     *
     * <p>The aim is to handle the exception possibly raised by the underlying smartcard.io method.
     *
     * @return An empty list if no reader is available.
     * @throws PluginIOException If an error occurs while accessing the list.
     */
    const std::vector<std::shared_ptr<CardTerminal>> getCardTerminalList() const;
};

}
}
}