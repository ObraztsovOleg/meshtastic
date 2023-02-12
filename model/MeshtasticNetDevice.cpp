#include "./MeshtasticNetDevice.h"

#include "ns3/log.h"

namespace ns3
{
namespace meshtastic
{

// NS_LOG_COMPONENT_DEFINE("MechtasticMac");

MeshtasticNetDevice::MeshtasticNetDevice()
{
}

MeshtasticNetDevice::~MeshtasticNetDevice()
{
}

void
MeshtasticNetDevice::Send (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Send the packet to the MAC layer, if it exists
  handleToRadio(const uint8_t *buf, size_t bufLength);
}

bool
MeshtasticNetDevice::Send (Ptr<Packet> packet, const Address& dest,
                     uint16_t protocolNumber)

{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  // Fallback to the vanilla Send method
  Send (packet);

  return true;
}

} // namespace meshtastic
} // namespace ns3