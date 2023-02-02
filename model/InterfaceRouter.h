#pragma once

#include "ns3/end-device-lorawan-mac.h"
#include "ns3/simple-end-device-lora-phy.h"
#include "ns3/lora-frame-header.h"

namespace ns3
{
namespace meshtastic
{

class InterfaceRouter : public lorawan::SimpleEndDeviceLoraPhy,
                        public lorawan::EndDeviceLorawanMac,
{
  public:
    // RadioLibInterface(RADIOLIB_PIN_TYPE cs, RADIOLIB_PIN_TYPE irq, RADIOLIB_PIN_TYPE rst,
    // RADIOLIB_PIN_TYPE busy, SPIClass &spi,
    //                   PhysicalLayer *iface = NULL);

    // Implementation of LoraPhy's pure virtual functions
    virtual void DoSend(Ptr<Packet> packet) override;
    virtual void Send(Ptr<Packet> packet,
                      lorawan::LoraTxParameters txParams,
                      double frequencyMHz,
                      double txPowerDbm) override;

    // protected:
    //   Ptr<lorawan::SimpleEndDeviceLoraPhy> m_endDevice;
  protected:
    /**
     * Structure representing the parameters that will be used in the
     * retransmission procedure.
     */

  private:
    /**
     * The message type to apply to packets sent with the Send method.
     */
    lorawan::LorawanMacHeader::MType m_mType;

    uint16_t m_currentFCnt;
};

} // namespace meshtastic

} // namespace ns3
