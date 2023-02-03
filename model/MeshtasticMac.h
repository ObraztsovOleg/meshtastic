#pragma once

#include "firmware/src/mesh/RadioLibInterface.h"

#include "ns3/simple-end-device-lora-phy.h"

namespace ns3
{
namespace meshtastic
{

class MeshtasticMac : public SimpleEndDeviceLoraPhy, public RadioLibInterface, public RadioInterface
{
  public:
    MeshtasticMac();
    virtual ~MeshtasticMac();

    void startSend(meshtastic_MeshPacket* txp) override;
    void Send (Ptr<Packet> packet, LoraTxParameters txParams,
                              double frequencyMHz, double txPowerDbm) override;

  protected:
    bool m_headerDisabled;

    int16_t m_sendRes;
};

} // namespace meshtastic

} // namespace ns3
