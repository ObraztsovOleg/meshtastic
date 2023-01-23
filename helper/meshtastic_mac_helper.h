#ifndef MESHTASTIC_MAC_HELPER_H
#define MESHTASTIC_MAC_HELPER_H

#include "ns3/net-device.h"
#include "ns3/lora-channel.h"
#include "ns3/lora-phy.h"
#include "ns3/meshtastic_mac.h"
#include "ns3/class_a_end_device_meshtastic_mac.h"
#include "ns3/lora-device-address-generator.h"
#include "ns3/gateway_meshtastic_mac.h"
#include "ns3/node-container.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {
namespace meshtastic {

class MeshtasticMacHelper
{
public:
  /**
   * Define the kind of device. Can be either GW (Gateway) or ED (End Device).
   */
  enum DeviceType { GW, ED_A };

  /**
   * Define the operational region.
   */
  enum Regions {
    EU,
    US,
    China,
    EU433MHz,
    Australia,
    CN,
    AS923MHz,
    SouthKorea,
    SingleChannel,
    ALOHA
  };

  /**
   * Create a mac helper without any parameter set. The user must set
   * them all to be able to call Install later.
   */
  MeshtasticMacHelper ();

  /**
   * Set an attribute of the underlying MAC object.
   *
   * \param name the name of the attribute to set.
   * \param v the value of the attribute.
   */
  void Set (std::string name, const AttributeValue &v);

  /**
   * Set the address generator to use for creation of these nodes.
   */
  void SetAddressGenerator (Ptr<lorawan::LoraDeviceAddressGenerator> addrGen);

  /**
   * Set the kind of MAC this helper will create.
   *
   * \param dt the device type (either gateway or end device).
   */
  void SetDeviceType (enum DeviceType dt);

  /**
   * Set the region in which the device is to operate.
   */
  void SetRegion (enum Regions region);

  /**
   * Create the LorawanMac instance and connect it to a device
   *
   * \param node the node on which we wish to create a wifi MAC.
   * \param device the device within which this MAC will be created.
   * \returns a newly-created LorawanMac object.
   */
  Ptr<MeshtasticMac> Create (Ptr<Node> node, Ptr<NetDevice> device) const;

  /**
   * Set up the end device's data rates
   * This function assumes we are using the following convention:
   * SF7 -> DR5
   * SF8 -> DR4
   * SF9 -> DR3
   * SF10 -> DR2
   * SF11 -> DR1
   * SF12 -> DR0
   */
  static std::vector<int> SetSpreadingFactorsUp (NodeContainer endDevices, NodeContainer gateways,
                                                 Ptr<lorawan::LoraChannel> channel);
  /**
   * Set up the end device's data rates according to the given distribution.
   */
  static std::vector<int> SetSpreadingFactorsGivenDistribution (NodeContainer endDevices,
                                                                NodeContainer gateways,
                                                                std::vector<double> distribution);

private:
  /**
   * Perform region-specific configurations for the 868 MHz EU band.
   */
  void ConfigureForEuRegion (Ptr<ClassAEndDeviceMeshtasticMac> edMac) const;

  /**
   * Perform region-specific configurations for the 868 MHz EU band.
   */
  void ConfigureForEuRegion (Ptr<GatewayMeshtasticMac> gwMac) const;

  /**
   * Apply configurations that are common both for the GatewayLorawanMac and the
   * ClassAEndDeviceLorawanMac classes.
   */
  void ApplyCommonEuConfigurations (Ptr<MeshtasticMac> MeshtasticMac) const;

  /**
   * Perform region-specific configurations for the SINGLECHANNEL band.
   */
  void ConfigureForSingleChannelRegion (Ptr<ClassAEndDeviceMeshtasticMac> edMac) const;

  /**
   * Perform region-specific configurations for the SINGLECHANNEL band.
   */
  void ConfigureForSingleChannelRegion (Ptr<GatewayMeshtasticMac> gwMac) const;

  /**
   * Apply configurations that are common both for the GatewayLorawanMac and the
   * ClassAEndDeviceLorawanMac classes.
   */
  void ApplyCommonSingleChannelConfigurations (Ptr<MeshtasticMac> MeshtasticMac) const;

  /**
   * Perform region-specific configurations for the ALOHA band.
   */
  void ConfigureForAlohaRegion (Ptr<ClassAEndDeviceMeshtasticMac> edMac) const;

  /**
   * Perform region-specific configurations for the ALOHA band.
   */
  void ConfigureForAlohaRegion (Ptr<GatewayMeshtasticMac> gwMac) const;

  /**
   * Apply configurations that are common both for the GatewayLorawanMac and the
   * ClassAEndDeviceLorawanMac classes.
   */
  void ApplyCommonAlohaConfigurations (Ptr<MeshtasticMac> MeshtasticMac) const;

  ObjectFactory m_mac;
  Ptr<lorawan::LoraDeviceAddressGenerator> m_addrGen; //!< Pointer to the address generator to use
  enum DeviceType m_deviceType; //!< The kind of device to install
  enum Regions m_region; //!< The region in which the device will operate
};

} // namespace meshtastic

} // namespace ns3
#endif /* MESHTASTIC_MAC_HELPER_H */
