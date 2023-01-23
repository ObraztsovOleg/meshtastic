#ifndef MESHTASTIC_HELPER_H
#define MESHTASTIC_HELPER_H

#include "ns3/lora-net-device.h"
#include "ns3/lora-packet-tracker.h"
#include "ns3/lora-phy-helper.h"
#include "ns3/meshtastic_mac_helper.h"
#include "ns3/net-device-container.h"

// #include "ns3/net-device.h"
// #include "ns3/node-container.h"

// #include <ctime>

namespace ns3
{
namespace meshtastic
{

class LoraMeshHelper
{
  public:
    ~LoraMeshHelper();
    LoraMeshHelper();

    NetDeviceContainer Install(const LoraMeshHelper& meshHelper,
                               const lorawan::LoraPhyHelper& phyHelper,
                               const MeshtasticMacHelper& macHelper,
                               NodeContainer c) const;
    NetDeviceContainer Install(const LoraMeshHelper& meshHelper,
                               const lorawan::LoraPhyHelper& phyHelper,
                               const MeshtasticMacHelper& macHelper,
                               Ptr<Node> node) const;

    lorawan::LoraPacketTracker* m_packetTracker = 0;
};

} // namespace meshtastic

} // namespace ns3

#endif /* MESHTASTIC_HELPER_H */
