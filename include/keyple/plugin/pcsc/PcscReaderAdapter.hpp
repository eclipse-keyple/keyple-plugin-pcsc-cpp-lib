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

#include <atomic>
#include <memory>
#include <typeinfo>

#include "keyple/core/plugin/spi/reader/ConfigurableReaderSpi.hpp"
#include "keyple/core/plugin/spi/reader/observable/ObservableReaderSpi.hpp"
#include "keyple/core/plugin/spi/reader/observable/state/insertion/CardInsertionWaiterBlockingSpi.hpp"
#include "keyple/core/plugin/spi/reader/observable/state/processing/CardPresenceMonitorBlockingSpi.hpp"
#include "keyple/core/plugin/spi/reader/observable/state/removal/CardRemovalWaiterBlockingSpi.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/cpp/Card.hpp"
#include "keyple/plugin/pcsc/cpp/CardChannel.hpp"
#include "keyple/plugin/pcsc/cpp/CardTerminal.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::plugin::spi::reader::ConfigurableReaderSpi;
using keyple::core::plugin::spi::reader::observable::ObservableReaderSpi;
using keyple::core::plugin::spi::reader::observable::state::insertion::CardInsertionWaiterBlockingSpi;
using keyple::core::plugin::spi::reader::observable::state::processing::CardPresenceMonitorBlockingSpi;
using keyple::core::plugin::spi::reader::observable::state::removal::CardRemovalWaiterBlockingSpi;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::plugin::pcsc::cpp::Card;
using keyple::plugin::pcsc::cpp::CardChannel;
using keyple::plugin::pcsc::cpp::CardTerminal;

using DisconnectionMode = PcscReader::DisconnectionMode;

class PcscPluginAdapter;

/**
 * Implementation of PcscReaderAdapter.
 *
 * @since 2.0.0
 */
class PcscReaderAdapter final
: public PcscReader,
  public ConfigurableReaderSpi,
  public ObservableReaderSpi,
  public CardInsertionWaiterBlockingSpi,
  public CardPresenceMonitorBlockingSpi,
  public CardRemovalWaiterBlockingSpi {
public:
    /**
     * Constructor.
     *
     * @since 2.0.0
     */
    PcscReaderAdapter(std::shared_ptr<CardTerminal> terminal,
                      std::shared_ptr<PcscPluginAdapter> pluginAdapter,
                      const int cardMonitoringCycleDuration);
    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    waitForCardInsertion() override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    stopWaitForCardInsertion() override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    bool
    isProtocolSupported(const std::string& readerProtocol) const final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    activateProtocol(const std::string& readerProtocol) override;


    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    deactivateProtocol(const std::string& readerProtocol) final;

    /**
     * {@inheritDoc}
     *
     * @throws PatternSyntaxException If the expression's syntax is invalid
     * @since 2.0.0
     */
    bool
    isCurrentProtocol(const std::string& readerProtocol) const final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    onStartDetection() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    onStopDetection() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string&
    getName() const final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    openPhysicalChannel() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    closePhysicalChannel() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    bool
    isPhysicalChannelOpen() const final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    bool
    checkCardPresence() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string
    getPowerOnData() const final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::vector<uint8_t>
    transmitApdu(const std::vector<uint8_t>& apduCommandData) final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    bool
    isContactless() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    onUnregister() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    monitorCardPresenceDuringProcessing() override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    stopCardPresenceMonitoringDuringProcessing() override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    waitForCardRemoval() final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void
    stopWaitForCardRemoval() final;

    /**
     * {@inheritDoc}
     *
     * <p>The default value is {@link SharingMode#EXCLUSIVE}.
     *
     * @since 2.0.0
     */
    PcscReader&
    setSharingMode(const SharingMode sharingMode) final;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    PcscReader&
    setContactless(const bool contactless) final;

    /**
     * {@inheritDoc}
     *
     * <p>The default value is {@link IsoProtocol#ANY}.
     *
     * @since 2.0.0
     */
    PcscReader&
    setIsoProtocol(const IsoProtocol& isoProtocol) final;

    /**
     * {@inheritDoc}
     *
     * <p>The default value is {@link DisconnectionMode#LEAVE}.
     *
     * @since 2.0.0
     */
    PcscReader&
    setDisconnectionMode(const DisconnectionMode disconnectionMode) final;

    /**
     * {@inheritDoc}
     *
     * @since 2.1.0
     */
    const std::vector<uint8_t>
    transmitControlCommand(
        const int commandId, const std::vector<uint8_t>& command) override;

    /**
     * {@inheritDoc}
     *
     * @since 2.1.0
     */
    int
    getIoctlCcidEscapeCommandId() const override;

private:
    /**
     * C++ specific
     */
    bool mIsInitialized;

    /**
     * C++ specific
     */
    bool mIsPhysicalChannelOpen;

    /**
     *
     */
    const std::unique_ptr<Logger> mLogger = LoggerFactory::getLogger(typeid(PcscReaderAdapter));

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
    std::shared_ptr<PcscPluginAdapter> mPluginAdapter;

    /**
     *
     */
    const int mCardMonitoringCycleDuration;

    /**
     *
     */
    const std::vector<std::uint8_t> mPingApdu;

    /**
     *
     */
    std::shared_ptr<Card> mCard;

    /**
     *
     */
    std::shared_ptr<CardChannel> mChannel;

    /**
     *
     */
    bool mIsWindows;

    /**
     *
     */
    bool mIsContactless;

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
     *
     */
    std::atomic<bool> mLoopWaitCard;

    /**
     *
     */
    std::atomic<bool> mLoopWaitCardRemoval;

    /**
     *
     */
    bool mIsObservationActive;


    /**
     *
     */
    void
    closePhysicalChannelSafely();

    /**
     *
     */
    void
    resetContext();

    /**
    * Disconnects the current card and resets the context and reader state.
    *
    * <p>This method handles the disconnection of a card, taking into account
    * the specific disconnection mode. If the card is an instance of JnaCard, it
    * disconnects using the extended mode specified by
    * getDisposition(DisconnectionMode)} and resets the reader state to avoid
    * incorrect card detection in subsequent operations. For other card types,
    * it disconnects using the specified disconnection mode directly.
    *
    * <p>If a CardException occurs during the operation, a ReaderIOException is
    * thrown with the associated error message.
    *
    * <p>Once the disconnection is handled, the method ensures that the context
    * is reset.
    *
    * @throw ReaderIOException If an error occurs while closing the physical
    * channel.
    */
    void
    disconnect();

    /**
     * Maps a DisconnectionMode to the corresponding SCARD_* constant.
     *
     * @param mode The disconnection mode.
     * @return The corresponding SCARD_* value.
     */
    static int getDisposition(const DisconnectionMode mode);

    /**
    * Resets the state of the card reader.
    *
    * <p>This method attempts to reset the reader state based on the current
    * disconnection mode. If the disconnection mode is set to UNPOWER, it
    * reconnects to the terminal and then disconnects without powering off the
    * reader. If any {@link CardException} occurs during this process, it is
    * handled silently.
    */
    void
    resetReaderState();

    /**
     *
     */
    void
    waitForCardRemovalByPolling();

    /**
     *
     */
    void
    waitForCardRemovalStandard();
};

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
