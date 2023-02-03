#include "MechtasticMac.h"

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
MeshtasticMac::Send (Ptr<Packet> packet, LoraTxParameters txParams,
                              double frequencyMHz, double txPowerDbm)
{
  NS_LOG_FUNCTION (this << packet << txParams << frequencyMHz << txPowerDbm);

  NS_LOG_INFO ("Current state: " << m_state);

  // We must be either in STANDBY or SLEEP mode to send a packet
  if (m_state != STANDBY && m_state != SLEEP)
    {
      NS_LOG_INFO ("Cannot send because device is currently not in STANDBY or SLEEP mode");
      m_sendRes = RADIOLIB_ERR_NONE;
      return;
    }

  // Compute the duration of the transmission
  Time duration = GetOnAirTime (packet, txParams);

  // We can send the packet: switch to the TX state
  SwitchToTx (txPowerDbm);

  // Tag the packet with information about its Spreading Factor
  LoraTag tag;
  packet->RemovePacketTag (tag);
  tag.SetSpreadingFactor (txParams.sf);
  packet->AddPacketTag (tag);

  // Send the packet over the channel
  NS_LOG_INFO ("Sending the packet in the channel");
  m_channel->Send (this, packet, txPowerDbm, txParams, duration, frequencyMHz);

  // Schedule the switch back to STANDBY mode.
  // For reference see SX1272 datasheet, section 4.1.6
  Simulator::Schedule (duration, &EndDeviceLoraPhy::SwitchToStandby, this);

  // Schedule the txFinished callback, if it was set
  // The call is scheduled just after the switch to standby in case the upper
  // layer wishes to change the state. This ensures that it will find a PHY in
  // STANDBY mode.
  if (!m_txFinishedCallback.IsNull ())
    {
      Simulator::Schedule (duration + NanoSeconds (10),
                           &SimpleEndDeviceLoraPhy::m_txFinishedCallback, this,
                           packet);
    }


  // Call the trace source
  if (m_device)
    {
      m_startSending (packet, m_device->GetNode ()->GetId ());
    }
  else
    {
      m_startSending (packet, 0);
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

} // namespace meshtastic
} // namespace ns3