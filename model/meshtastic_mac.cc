#include "ns3/meshtastic_mac.h"
#include "ns3/log.h"

namespace ns3 {
namespace meshtastic {

NS_LOG_COMPONENT_DEFINE ("MeshtasticMac");

NS_OBJECT_ENSURE_REGISTERED (MeshtasticMac);

TypeId
MeshtasticMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MeshtasticMac")
    .SetParent<Object> ()
    .SetGroupName ("mestastic")
    .AddTraceSource ("SentNewPacket",
                     "Trace source indicating a new packet "
                     "arrived at the MAC layer",
                     MakeTraceSourceAccessor (&MeshtasticMac::m_sentNewPacket),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("ReceivedPacket",
                     "Trace source indicating a packet "
                     "was correctly received at the MAC layer",
                     MakeTraceSourceAccessor (&MeshtasticMac::m_receivedPacket),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("CannotSendBecauseDutyCycle",
                     "Trace source indicating a packet "
                     "could not be sent immediately because of duty cycle limitations",
                     MakeTraceSourceAccessor (&MeshtasticMac::m_cannotSendBecauseDutyCycle),
                     "ns3::Packet::TracedCallback");
  return tid;
}

MeshtasticMac::MeshtasticMac ()
{
  NS_LOG_FUNCTION (this);
}

MeshtasticMac::~MeshtasticMac ()
{
  NS_LOG_FUNCTION (this);
}

void
MeshtasticMac::SetDevice (Ptr<NetDevice> device)
{
  m_device = device;
}

Ptr<NetDevice>
MeshtasticMac::GetDevice (void)
{
  return m_device;
}

Ptr<lorawan::LoraPhy>
MeshtasticMac::GetPhy (void)
{
  return m_phy;
}

void
MeshtasticMac::SetPhy (Ptr<lorawan::LoraPhy> phy)
{
  // Set the phy
  m_phy = phy;

  // Connect the receive callbacks
  m_phy->SetReceiveOkCallback (MakeCallback (&MeshtasticMac::Receive, this));
  m_phy->SetReceiveFailedCallback (MakeCallback (&MeshtasticMac::FailedReception, this));
  m_phy->SetTxFinishedCallback (MakeCallback (&MeshtasticMac::TxFinished, this));
}

lorawan::LogicalLoraChannelHelper
MeshtasticMac::GetLogicalLoraChannelHelper (void)
{
  return m_channelHelper;
}

void
MeshtasticMac::SetLogicalLoraChannelHelper (lorawan::LogicalLoraChannelHelper helper)
{
  m_channelHelper = helper;
}

uint8_t
MeshtasticMac::GetSfFromDataRate (uint8_t dataRate)
{
  NS_LOG_FUNCTION (this << unsigned(dataRate));

  // Check we are in range
  if (dataRate >= m_sfForDataRate.size ())
    {
      return 0;
    }

  return m_sfForDataRate.at (dataRate);
}

double
MeshtasticMac::GetBandwidthFromDataRate (uint8_t dataRate)
{
  NS_LOG_FUNCTION (this << unsigned(dataRate));

  // Check we are in range
  if (dataRate > m_bandwidthForDataRate.size ())
    {
      return 0;
    }

  return m_bandwidthForDataRate.at (dataRate);
}

double
MeshtasticMac::GetDbmForTxPower (uint8_t txPower)
{
  NS_LOG_FUNCTION (this << unsigned (txPower));

  if (txPower > m_txDbmForTxPower.size ())
    {
      return 0;
    }

  return m_txDbmForTxPower.at (txPower);
}

void
MeshtasticMac::SetSfForDataRate (std::vector<uint8_t> sfForDataRate)
{
  m_sfForDataRate = sfForDataRate;
}

void
MeshtasticMac::SetBandwidthForDataRate (std::vector<double> bandwidthForDataRate)
{
  m_bandwidthForDataRate = bandwidthForDataRate;
}

void
MeshtasticMac::SetMaxAppPayloadForDataRate (std::vector<uint32_t> maxAppPayloadForDataRate)
{
  m_maxAppPayloadForDataRate = maxAppPayloadForDataRate;
}

void
MeshtasticMac::SetTxDbmForTxPower (std::vector<double> txDbmForTxPower)
{
  m_txDbmForTxPower = txDbmForTxPower;
}

void
MeshtasticMac::SetNPreambleSymbols (int nPreambleSymbols)
{
  m_nPreambleSymbols = nPreambleSymbols;
}

int
MeshtasticMac::GetNPreambleSymbols (void)
{
  return m_nPreambleSymbols;
}

void
MeshtasticMac::SetReplyDataRateMatrix (ReplyDataRateMatrix replyDataRateMatrix)
{
  m_replyDataRateMatrix = replyDataRateMatrix;
}
}
}
