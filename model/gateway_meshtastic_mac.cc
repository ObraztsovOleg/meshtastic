#include "ns3/gateway_meshtastic_mac.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-frame-header.h"
#include "ns3/log.h"

namespace ns3 {
namespace meshtastic {

NS_LOG_COMPONENT_DEFINE ("GatewayMeshtasticMac");

NS_OBJECT_ENSURE_REGISTERED (GatewayMeshtasticMac);

TypeId
GatewayMeshtasticMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GatewayMeshtasticMac")
    .SetParent<MeshtasticMac> ()
    .AddConstructor<GatewayMeshtasticMac> ()
    .SetGroupName ("lorawan");
  return tid;
}

GatewayMeshtasticMac::GatewayMeshtasticMac ()
{
  NS_LOG_FUNCTION (this);
}

GatewayMeshtasticMac::~GatewayMeshtasticMac ()
{
  NS_LOG_FUNCTION (this);
}

void
GatewayMeshtasticMac::Send (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Get DataRate to send this packet with
  lorawan::LoraTag tag;
  packet->RemovePacketTag (tag);
  uint8_t dataRate = tag.GetDataRate ();
  double frequency = tag.GetFrequency ();
  NS_LOG_DEBUG ("DR: " << unsigned (dataRate));
  NS_LOG_DEBUG ("SF: " << unsigned (GetSfFromDataRate (dataRate)));
  NS_LOG_DEBUG ("BW: " << GetBandwidthFromDataRate (dataRate));
  NS_LOG_DEBUG ("Freq: " << frequency << " MHz");
  packet->AddPacketTag (tag);

  // Make sure we can transmit this packet
  if (m_channelHelper.GetWaitingTime(CreateObject<lorawan::LogicalLoraChannel> (frequency)) > Time(0))
    {
      // We cannot send now!
      NS_LOG_WARN ("Trying to send a packet but Duty Cycle won't allow it. Aborting.");
      return;
    }

  lorawan::LoraTxParameters params;
  params.sf = GetSfFromDataRate (dataRate);
  params.headerDisabled = false;
  params.codingRate = 1;
  params.bandwidthHz = GetBandwidthFromDataRate (dataRate);
  params.nPreamble = 8;
  params.crcEnabled = 1;
  params.lowDataRateOptimizationEnabled = lorawan::LoraPhy::GetTSym (params) > MilliSeconds (16) ? true : false;

  // Get the duration
  Time duration = m_phy->GetOnAirTime (packet, params);

  NS_LOG_DEBUG ("Duration: " << duration.GetSeconds ());

  // Find the channel with the desired frequency
  double sendingPower = m_channelHelper.GetTxPowerForChannel
      (CreateObject<lorawan::LogicalLoraChannel> (frequency));

  // Add the event to the channelHelper to keep track of duty cycle
  m_channelHelper.AddEvent (duration, CreateObject<lorawan::LogicalLoraChannel>
                              (frequency));

  // Send the packet to the PHY layer to send it on the channel
  m_phy->Send (packet, params, frequency, sendingPower);

  m_sentNewPacket (packet);
}

bool
GatewayMeshtasticMac::IsTransmitting (void)
{
  return m_phy->IsTransmitting ();
}

void
GatewayMeshtasticMac::Receive (Ptr<Packet const> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Make a copy of the packet to work on
  Ptr<Packet> packetCopy = packet->Copy ();

  // Only forward the packet if it's uplink
  lorawan::LorawanMacHeader macHdr;
  packetCopy->PeekHeader (macHdr);

  if (macHdr.IsUplink ())
    {
      m_device->GetObject<lorawan::LoraNetDevice> ()->Receive (packetCopy);

      NS_LOG_DEBUG ("Received packet: " << packet);

      m_receivedPacket (packet);
    }
  else
    {
      NS_LOG_DEBUG ("Not forwarding downlink message to NetDevice");
    }
}

void
GatewayMeshtasticMac::FailedReception (Ptr<Packet const> packet)
{
  NS_LOG_FUNCTION (this << packet);
}

void
GatewayMeshtasticMac::TxFinished (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION_NOARGS ();
}

Time
GatewayMeshtasticMac::GetWaitingTime (double frequency)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_channelHelper.GetWaitingTime (CreateObject<lorawan::LogicalLoraChannel>
                                           (frequency));
}
}
}
