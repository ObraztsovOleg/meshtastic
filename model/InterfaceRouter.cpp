#include "InterfaceRouter.h"
#include "ns3/log.h"
#include "ns3/lora-tag.h"
// #include "ns3/lora-phy.h"

namespace ns3
{
namespace meshtastic
{

NS_LOG_COMPONENT_DEFINE("InterfaceRouter");

void
InterfaceRouter::DoSend(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);
    // Checking if this is the transmission of a new packet
    if (packet != m_retxParams.packet)
    {
        NS_LOG_DEBUG(
            "Received a new packet from application. Resetting retransmission parameters.");
        m_currentFCnt++;
        NS_LOG_DEBUG("APP packet: " << packet << ".");

        // Add the Lora Frame Header to the packet
        lorawan::LoraFrameHeader frameHdr;
        ApplyNecessaryOptions(frameHdr);
        packet->AddHeader(frameHdr);

        NS_LOG_INFO("Added frame header of size " << frameHdr.GetSerializedSize() << " bytes.");

        // Check that MACPayload length is below the allowed maximum
        if (packet->GetSize() > m_maxAppPayloadForDataRate.at(m_dataRate))
        {
            NS_LOG_WARN("Attempting to send a packet larger than the maximum allowed"
                        << " size at this DataRate (DR" << unsigned(m_dataRate)
                        << "). Transmission canceled.");
            return;
        }

        // Add the Lora Mac header to the packet
        lorawan::LorawanMacHeader macHdr;
        ApplyNecessaryOptions(macHdr);
        packet->AddHeader(macHdr);

        // Reset MAC command list
        m_macCommandList.clear();

        if (m_retxParams.waitingAck)
        {
            // Call the callback to notify about the failure
            uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
            m_requiredTxCallback(txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
            NS_LOG_DEBUG(" Received new packet from the application layer: stopping retransmission "
                         "procedure. Used "
                         << unsigned(txs) << " transmissions out of a maximum of "
                         << unsigned(m_maxNumbTx) << ".");
        }

        // Reset retransmission parameters
        resetRetransmissionParameters();

        // If this is the first transmission of a confirmed packet, save parameters for the
        // (possible) next retransmissions.
        if (m_mType == lorawan::LorawanMacHeader::CONFIRMED_DATA_UP)
        {
            m_retxParams.packet = packet->Copy();
            m_retxParams.retxLeft = m_maxNumbTx;
            m_retxParams.waitingAck = true;
            m_retxParams.firstAttempt = Simulator::Now();
            m_retxParams.retxLeft =
                m_retxParams.retxLeft - 1; // decreasing the number of retransmissions

            NS_LOG_DEBUG("Message type is " << m_mType);
            NS_LOG_DEBUG("It is a confirmed packet. Setting retransmission parameters and "
                         "decreasing the number of transmissions left.");

            NS_LOG_INFO("Added MAC header of size " << macHdr.GetSerializedSize() << " bytes.");

            // Sent a new packet
            NS_LOG_DEBUG("Copied packet: " << m_retxParams.packet);
            m_sentNewPacket(m_retxParams.packet);

            // static_cast<ClassAEndDeviceLorawanMac*>(this)->SendToPhy (m_retxParams.packet);
            SendToPhy(m_retxParams.packet);
        }
        else
        {
            m_sentNewPacket(packet);
            // static_cast<ClassAEndDeviceLorawanMac*>(this)->SendToPhy (packet);
            SendToPhy(packet);
        }
    }
    // this is a retransmission
    else
    {
        if (m_retxParams.waitingAck)
        {
            // Remove the headers
            lorawan::LorawanMacHeader macHdr;
            lorawan::LoraFrameHeader frameHdr;
            packet->RemoveHeader(macHdr);
            packet->RemoveHeader(frameHdr);

            // Add the Lora Frame Header to the packet
            frameHdr = lorawan::LoraFrameHeader();
            ApplyNecessaryOptions(frameHdr);
            packet->AddHeader(frameHdr);

            NS_LOG_INFO("Added frame header of size " << frameHdr.GetSerializedSize() << " bytes.");

            // Add the Lorawan Mac header to the packet
            macHdr = lorawan::LorawanMacHeader();
            ApplyNecessaryOptions(macHdr);
            packet->AddHeader(macHdr);
            m_retxParams.retxLeft =
                m_retxParams.retxLeft - 1; // decreasing the number of retransmissions
            NS_LOG_DEBUG("Retransmitting an old packet.");

            // static_cast<ClassAEndDeviceLorawanMac*>(this)->SendToPhy (m_retxParams.packet);
            SendToPhy(m_retxParams.packet);
        }
    }
}

void
InterfaceRouter::Send(Ptr<Packet> packet,
                      lorawan::LoraTxParameters txParams,
                      double frequencyMHz,
                      double txPowerDbm)
{
    txParams.sf = sf;                 //!< Spreading Factor
    txParams.codingRate = cr;         //!< Code rate (obtained as 4/(codingRate+4))
    txParams.bandwidthHz = bw * 1000; //!< Bandwidth in Hz

    NS_LOG_FUNCTION(this << packet << txParams << frequencyMHz << txPowerDbm);

    NS_LOG_INFO("Current state: " << m_state);

    // We must be either in STANDBY or SLEEP mode to send a packet
    if (m_state != STANDBY && m_state != SLEEP)
    {
        NS_LOG_INFO("Cannot send because device is currently not in STANDBY or SLEEP mode");
        return;
    }

    // Compute the duration of the transmission
    Time duration = GetOnAirTime(packet, txParams);

    // We can send the packet: switch to the TX state
    SwitchToTx(txPowerDbm);

    // Tag the packet with information about its Spreading Factor
    lorawan::LoraTag tag;
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
                            &InterfaceRouter::m_txFinishedCallback,
                            this,
                            packet);
    }

    // Call the trace source
    // if (m_device)
    // {
    //     m_startSending(packet, m_device->GetNode()->GetId());
    // }
    // else
    // {
    //     m_startSending(packet, 0);
    // }
}

} // namespace meshtastic
} // namespace ns3