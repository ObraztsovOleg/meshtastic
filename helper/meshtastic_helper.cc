#include "ns3/meshtastic_helper.h"

#include "ns3/log.h"

// #include <fstream>

namespace ns3
{
namespace meshtastic
{

NS_LOG_COMPONENT_DEFINE("LoraMeshHelper");

LoraMeshHelper::LoraMeshHelper()
{
}

LoraMeshHelper::~LoraMeshHelper()
{
}

NetDeviceContainer
LoraMeshHelper::Install(const LoraMeshHelper& meshHelpe,
                        const lorawan::LoraPhyHelper& phyHelper,
                        const MeshtasticMacHelper& macHelper,
                        NodeContainer c) const
{
    NS_LOG_FUNCTION_NOARGS();

    NetDeviceContainer devices;

    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<lorawan::LoraNetDevice> device = CreateObject<lorawan::LoraNetDevice>();

        // Create the PHY
        Ptr<lorawan::LoraPhy> phy = phyHelper.Create(node, device);
        NS_ASSERT(phy != 0);
        device->SetPhy(phy);
        NS_LOG_DEBUG("Done creating the PHY");

        if (m_packetTracker)
        {
            if (phyHelper.GetDeviceType() == TypeId::LookupByName("ns3::SimpleEndDeviceLoraPhy"))
            {
                phy->TraceConnectWithoutContext(
                    "StartSending",
                    MakeCallback(&(lorawan::LoraPacketTracker::TransmissionCallback),
                                 m_packetTracker));
            }
            else if (phyHelper.GetDeviceType() == TypeId::LookupByName("ns3::SimpleGatewayLoraPhy"))
            {
                phy->TraceConnectWithoutContext(
                    "StartSending",
                    MakeCallback(&(lorawan::LoraPacketTracker::TransmissionCallback),
                                 m_packetTracker));
                phy->TraceConnectWithoutContext(
                    "ReceivedPacket",
                    MakeCallback(&(lorawan::LoraPacketTracker::PacketReceptionCallback),
                                 m_packetTracker));
                phy->TraceConnectWithoutContext(
                    "LostPacketBecauseInterference",
                    MakeCallback(&(lorawan::LoraPacketTracker::InterferenceCallback),
                                 m_packetTracker));
                phy->TraceConnectWithoutContext(
                    "LostPacketBecauseNoMoreReceivers",
                    MakeCallback(&(lorawan::LoraPacketTracker::NoMoreReceiversCallback),
                                 m_packetTracker));
                phy->TraceConnectWithoutContext(
                    "LostPacketBecauseUnderSensitivity",
                    MakeCallback(&(lorawan::LoraPacketTracker::UnderSensitivityCallback),
                                 m_packetTracker));
                phy->TraceConnectWithoutContext(
                    "NoReceptionBecauseTransmitting",
                    MakeCallback(&(lorawan::LoraPacketTracker::LostBecauseTxCallback),
                                 m_packetTracker));
            }
        }

        // Create the MAC
        Ptr<MeshtasticMac> mac = macHelper.Create(node, device);
        NS_ASSERT(mac != 0);
        mac->SetPhy(phy);
        NS_LOG_DEBUG("Done creating the MAC");
        device->SetMac(mac);

        if (m_packetTracker)
        {
            if (phyHelper.GetDeviceType() == TypeId::LookupByName("ns3::SimpleEndDeviceLoraPhy"))
            {
                mac->TraceConnectWithoutContext(
                    "SentNewPacket",
                    MakeCallback(&(lorawan::LoraPacketTracker::MacTransmissionCallback), m_packetTracker));

                mac->TraceConnectWithoutContext(
                    "RequiredTransmissions",
                    MakeCallback(&(lorawan::LoraPacketTracker::RequiredTransmissionsCallback),
                                 m_packetTracker));
            }
            else if (phyHelper.GetDeviceType() == TypeId::LookupByName("ns3::SimpleGatewayLoraPhy"))
            {
                mac->TraceConnectWithoutContext(
                    "SentNewPacket",
                    MakeCallback(&(lorawan::LoraPacketTracker::MacTransmissionCallback), m_packetTracker));

                mac->TraceConnectWithoutContext(
                    "ReceivedPacket",
                    MakeCallback(&(lorawan::LoraPacketTracker::MacGwReceptionCallback), m_packetTracker));
            }
        }

        node->AddDevice(device);
        devices.Add(device);
        NS_LOG_DEBUG("node=" << node
                             << ", mob=" << node->GetObject<MobilityModel>()->GetPosition());
    }
    return devices;
}

NetDeviceContainer
LoraMeshHelper::Install(const LoraMeshHelper& mesh,
                        const lorawan::LoraPhyHelper& phy,
                        const MeshtasticMacHelper& mac,
                        Ptr<Node> node) const
{
    return Install(mesh, phy, mac, NodeContainer(node));
}

} // namespace meshtastic
} // namespace ns3
