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

#include <atomic>
#include <typeinfo>

/* Keyple Plugin Pcsc */
#include "AbstractPcscPluginAdapter.h"
#include "CardTerminal.h"
#include "ConfigurableReaderSpi.h"
#include "PcscReader.h"

/* Keyple Core Plugin */
#include "DontWaitForCardRemovalDuringProcessingSpi.h"
#include "ObservableReaderSpi.h"
#include "WaitForCardRemovalBlockingSpi.h"

/* Keyple Core Util */
#include "LoggerFactory.h"

namespace keyple {
namespace plugin {
namespace pcsc {

using namespace keyple::core::plugin::spi::reader::observable;
using namespace keyple::core::plugin::spi::reader::observable::state::processing;
using namespace keyple::core::plugin::spi::reader::observable::state::removal;
using namespace keyple::core::util::cpp;
using namespace keyple::plugin::pcsc::cpp;

/**
 * (package-private) <br>
 * Abstract class for all PC/SC reader adapters.
 *
 * @since 2.0.0
 */
class AbstractPcscReaderAdapter
: public PcscReader,
  public ConfigurableReaderSpi,
  public ObservableReaderSpi,
  public DontWaitForCardRemovalDuringProcessingSpi,
  public WaitForCardRemovalBlockingSpi {
public:
    /**
     * (package-private)<br>
     * Creates an instance the class, keeps the terminal and parent plugin, extract the reader name
     * from the terminal.
     *
     * @param terminal The terminal from smartcard.io
     * @param pluginAdapter The reference to the parent plugin.
     * @since 2.0.0
     */
    AbstractPcscReaderAdapter(std::shared_ptr<CardTerminal> terminal,
                              std::shared_ptr<AbstractPcscPluginAdapter> pluginAdapter);

    /**
     *
     */
    virtual ~AbstractPcscReaderAdapter() = default;

    /**
     * (package-private)<br>
     * Gets the smartcard.io terminal.
     *
     * @return A not null reference.
     */
    std::shared_ptr<CardTerminal> getTerminal() const;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string& getName() const final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    bool isProtocolSupported(const std::string& readerProtocol) const final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void activateProtocol(const std::string& readerProtocol) final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void deactivateProtocol(const std::string& readerProtocol) final;

    /**
     * {@inheritDoc}
     *
     * @throws PatternSyntaxException If the expression's syntax is invalid
     * @since 2.0.0
     */
    bool isCurrentProtocol(const std::string& readerProtocol) const final;

    /**
     * {@inheritDoc}
     *
     * <p>The physical channel is open using the current sharing mode.
     *
     * @see #setSharingMode(SharingMode)
     * @since 2.0.0
     */
    void openPhysicalChannel() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void closePhysicalChannel() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    bool isPhysicalChannelOpen() const final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    bool checkCardPresence() final;

    /**
     * {@inheritDoc}
     *
     * <p>In the case of a PC/SC reader, the power-on data is provided by the reader in the form of an
     * ATR ISO7816 structure whatever the card.
     *
     * @since 2.0.0
     */
    const std::string getPowerOnData() const final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::vector<uint8_t> transmitApdu(const std::vector<uint8_t>& apduCommandData) final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    bool isContactless() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void onUnregister() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void onStartDetection() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void onStopDetection() final;

    /**
     * {@inheritDoc}
     *
     * <p>The default value is {@link SharingMode#EXCLUSIVE}.
     *
     * @since 2.0.0
     */
    PcscReader& setSharingMode(const SharingMode sharingMode) final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    PcscReader& setContactless(const bool contactless) final;

    /**
     * {@inheritDoc}
     *
     * <p>The default value is {@link IsoProtocol#ANY}.
     *
     * @since 2.0.0
     */
    PcscReader& setIsoProtocol(const IsoProtocol& isoProtocol) final;

    /**
     * {@inheritDoc}
     *
     * <p>The default value is {@link DisconnectionMode#LEAVE}.
     *
     * @since 2.0.0
     */
    PcscReader& setDisconnectionMode(const DisconnectionMode disconnectionMode) final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void waitForCardRemoval() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void stopWaitForCardRemoval() final;

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger =
        LoggerFactory::getLogger(typeid(AbstractPcscReaderAdapter));

    /**
     *
     */
    std::shared_ptr<CardTerminal> mTerminal;

    /**
     *
     */
    const std::string mName;

    /**
     *
     */
    std::shared_ptr<AbstractPcscPluginAdapter> mPluginAdapter;

    /**
     *
     */
    bool mIsContactless;

    /**
     * C++ specific
     */
    bool mIsInitialized;

    /**
     *
     */
    bool mIsPhysicalChannelOpen;

    /**
     *
     */
    std::string mProtocol;

    /**
     *
     */
    bool mIsModeExclusive;

    /**
     *
     */
    DisconnectionMode mDisconnectionMode;

    /**
     * The latency delay value (in ms) determines the maximum time during which the
     * waitForCardAbsent blocking functions will execute.
     * This will correspond to the capacity to react to the interrupt signal of
     * the thread (see cancel method of the Future object)
     */
    static const long REMOVAL_LATENCY;

    /**
     *
     */
    std::atomic<bool> mLoopWaitCardRemoval;

};

}
}
}
