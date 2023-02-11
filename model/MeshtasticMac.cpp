#include "./MeshtasticMac.h"

#include "ns3/log.h"

// #include "ns3/packet.h"
// #include "meshpacket.h"
#include "ns3/lora-frame-header.h"

// #include "ns3/lora-frame-header.h"

namespace ns3
{
namespace meshtastic
{

// NS_LOG_COMPONENT_DEFINE("MechtasticMac");

MeshtasticMac::MeshtasticMac()
{
}

MeshtasticMac::~MeshtasticMac()
{
}

void
MeshtasticMac::Send(Ptr<Packet> packet,
                    LoraTxParameters txParams,
                    double frequencyMHz,
                    double txPowerDbm)
{
    NS_LOG_FUNCTION(this << packet << txParams << frequencyMHz << txPowerDbm);

    NS_LOG_INFO("Current state: " << m_state);

    // We must be either in STANDBY or SLEEP mode to send a packet
    if (m_state != STANDBY && m_state != SLEEP)
    {
        NS_LOG_INFO("Cannot send because device is currently not in STANDBY or SLEEP mode");
        m_sendRes = RADIOLIB_ERR_NONE;
        return;
    }

    // Compute the duration of the transmission
    Time duration = GetOnAirTime(packet, txParams);

    // We can send the packet: switch to the TX state
    SwitchToTx(txPowerDbm);

    // Tag the packet with information about its Spreading Factor
    LoraTag tag;
    packet->RemovePacketTag(tag);
    tag.SetSpreadingFactor(txParams.sf);
    packet->AddPacketTag(tag);

    // Send the packet over the channel
    NS_LOG_INFO("Sending the packet in the channel");
    m_channel->Send(this, packet, txPowerDbm, txParams, duration, frequencyMHz);

    // Schedule the switch back to STANDBY mode.
    // For reference see SX1272 datasheet, section 4.1.6
    Simulator::Schedule(duration, &EndDeviceLoraPhy::SwitchToStandby, this);

    // Schedule the txFinished callback, if it was set
    // The call is scheduled just after the switch to standby in case the upper
    // layer wishes to change the state. This ensures that it will find a PHY in
    // STANDBY mode.
    if (!m_txFinishedCallback.IsNull())
    {
        Simulator::Schedule(duration + NanoSeconds(10),
                            &SimpleEndDeviceLoraPhy::m_txFinishedCallback,
                            this,
                            packet);
    }

    // Call the trace source
    if (m_device)
    {
        m_startSending(packet, m_device->GetNode()->GetId());
    }
    else
    {
        m_startSending(packet, 0);
    }
}

void
MeshtasticMac::startSend(meshtastic_MeshPacket* txp)
{
    printPacket("Starting low level send", txp);
    if (disabled || !config.lora.tx_enabled)
    {
        LOG_WARN("startSend is dropping tx packet because we are disabled\n");
        packetPool.release(txp);
    }
    else
    {
        setStandby(); // Cancel any already in process receives

        configHardwareForSend(); // must be after setStandby

        size_t numbytes = beginSending(txp);
        Ptr<Packet> packet = Create<Packet>(radiobuf, numbytes);

        LoraTxParameters txParams;
        txParams.sf = sf;
        txParams.headerDisabled = m_headerDisabled;
        txParams.codingRate = cr;
        txParams.bandwidthHz = bw * 1000;
        txParams.nPreamble = preambleLength;
        txParams.crcEnabled = 1;
        txParams.lowDataRateOptimizationEnabled =
            LoraPhy::GetTSym(txParams) > MilliSeconds(16) ? true : false;

        double frequencyMHz = (double)savedFreq;
        double txPowerDbm = (double)power;

        Send(Ptr<Packet> packet, LoraTxParameters txParams, double frequencyMHz, double txPowerDbm);

        if (m_sendRes != RADIOLIB_ERR_NONE)
        {
            LOG_ERROR("startTransmit failed, error=%d\n", res);
            RECORD_CRITICALERROR(meshtastic_CriticalErrorCode_RADIO_SPI_BUG);

            // This send failed, but make sure to 'complete' it properly
            completeSending();
            startReceive(); // Restart receive mode (because startTransmit failed to put us in xmit
                            // mode)
        }

        // Must be done AFTER, starting transmit, because startTransmit clears (possibly stale)
        // interrupt pending register bits
        enableInterrupt(isrTxLevel0);
    }
}

void
MeshtasticMac::Send(Ptr<Packet> packet,
                    LoraTxParameters txParams,
                    double frequencyMHz,
                    double txPowerDbm)
{
    NS_LOG_FUNCTION(this << packet << frequencyMHz << txPowerDbm);

    // Get the time a packet with these parameters will take to be transmitted
    Time duration = GetOnAirTime(packet, txParams);

    NS_LOG_DEBUG("Duration of packet: " << duration << ", SF" << unsigned(txParams.sf));

    // Interrupt all receive operations
    std::list<Ptr<SimpleGatewayLoraPhy::ReceptionPath>>::iterator it;
    for (it = m_receptionPaths.begin(); it != m_receptionPaths.end(); ++it)
    {
        Ptr<SimpleGatewayLoraPhy::ReceptionPath> currentPath = *it;

        if (!currentPath->IsAvailable()) // Reception path is occupied
        {
            // Call the callback for reception interrupted by transmission
            // Fire the trace source
            if (m_device)
            {
                m_noReceptionBecauseTransmitting(currentPath->GetEvent()->GetPacket(),
                                                 m_device->GetNode()->GetId());
            }
            else
            {
                m_noReceptionBecauseTransmitting(currentPath->GetEvent()->GetPacket(), 0);
            }

            // Cancel the scheduled EndReceive call
            Simulator::Cancel(currentPath->GetEndReceive());

            // Free it
            // This also resets all parameters like packet and endReceive call
            currentPath->Free();
        }
    }

    // Send the packet in the channel
    m_channel->Send(this, packet, txPowerDbm, txParams, duration, frequencyMHz);

    Simulator::Schedule(duration, &SimpleGatewayLoraPhy::TxFinished, this, packet);

    m_isTransmitting = true;

    // Fire the trace source
    if (m_device)
    {
        m_startSending(packet, m_device->GetNode()->GetId());
    }
    else
    {
        m_startSending(packet, 0);
    }
}

void
MeshtasticMac::StartReceive(Ptr<Packet> packet,
                            double rxPowerDbm,
                            uint8_t sf,
                            Time duration,
                            double frequencyMHz)
{
    NS_LOG_FUNCTION(this << packet << rxPowerDbm << duration << frequencyMHz);

    // Fire the trace source
    m_phyRxBeginTrace(packet);

    if (m_isTransmitting)
    {
        // If we get to this point, there are no demodulators we can use
        NS_LOG_INFO("Dropping packet reception of packet with sf = "
                    << unsigned(sf) << " because we are in TX mode");

        m_phyRxEndTrace(packet);

        // Fire the trace source
        if (m_device)
        {
            m_noReceptionBecauseTransmitting(packet, m_device->GetNode()->GetId());
        }
        else
        {
            m_noReceptionBecauseTransmitting(packet, 0);
        }

        return;
    }

    // Add the event to the LoraInterferenceHelper
    Ptr<LoraInterferenceHelper::Event> event;
    event = m_interference.Add(duration, rxPowerDbm, sf, packet, frequencyMHz);

    // Cycle over the receive paths to check availability to receive the packet
    std::list<Ptr<SimpleGatewayLoraPhy::ReceptionPath>>::iterator it;

    for (it = m_receptionPaths.begin(); it != m_receptionPaths.end(); ++it)
    {
        Ptr<SimpleGatewayLoraPhy::ReceptionPath> currentPath = *it;

        // If the receive path is available and listening on the channel of
        // interest, we have a candidate
        if (currentPath->IsAvailable())
        {
            // See whether the reception power is above or below the sensitivity
            // for that spreading factor
            double sensitivity = SimpleGatewayLoraPhy::sensitivity[unsigned(sf) - 7];

            if (rxPowerDbm < sensitivity) // Packet arrived below sensitivity
            {
                NS_LOG_INFO("Dropping packet reception of packet with sf = "
                            << unsigned(sf) << " because under the sensitivity of " << sensitivity
                            << " dBm");

                if (m_device)
                {
                    m_underSensitivity(packet, m_device->GetNode()->GetId());
                }
                else
                {
                    m_underSensitivity(packet, 0);
                }

                // Since the packet is below sensitivity, it makes no sense to
                // search for another ReceivePath
                return;
            }
            else // We have sufficient sensitivity to start receiving
            {
                NS_LOG_INFO("Scheduling reception of a packet, "
                            << "occupying one demodulator");

                // Block this resource
                currentPath->LockOnEvent(event);
                m_occupiedReceptionPaths++;

                // Schedule the end of the reception of the packet
                EventId endReceiveEventId =
                    Simulator::Schedule(duration, &LoraPhy::EndReceive, this, packet, event);

                currentPath->SetEndReceive(endReceiveEventId);

                // Make sure we don't go on searching for other ReceivePaths
                return;
            }
        }
    }
    // If we get to this point, there are no demodulators we can use
    NS_LOG_INFO("Dropping packet reception of packet with sf = "
                << unsigned(sf) << " and frequency " << frequencyMHz
                << "MHz because no suitable demodulator was found");

    // Fire the trace source
    if (m_device)
    {
        m_noMoreDemodulators(packet, m_device->GetNode()->GetId());
    }
    else
    {
        m_noMoreDemodulators(packet, 0);
    }
}

} // namespace meshtastic
} // namespace ns3