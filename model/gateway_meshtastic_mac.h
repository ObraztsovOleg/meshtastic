#ifndef GATEWAY_MESHTASTIC_MAC_H
#define GATEWAY_MESHTASTIC_MAC_H

#include "ns3/meshtastic_mac.h"
#include "ns3/lora-tag.h"

namespace ns3 {
namespace meshtastic {

class GatewayMeshtasticMac : public MeshtasticMac
{
public:
  static TypeId GetTypeId (void);

  GatewayMeshtasticMac ();
  virtual ~GatewayMeshtasticMac ();

  // Implementation of the LorawanMac interface
  virtual void Send (Ptr<Packet> packet);

  // Implementation of the LorawanMac interface
  bool IsTransmitting (void);

  // Implementation of the LorawanMac interface
  virtual void Receive (Ptr<Packet const> packet);

  // Implementation of the LorawanMac interface
  virtual void FailedReception (Ptr<Packet const> packet);

  // Implementation of the LorawanMac interface
  virtual void TxFinished (Ptr<Packet const> packet);

  /**
   * Return the next time at which we will be able to transmit.
   *
   * \return The next transmission time.
   */
  Time GetWaitingTime (double frequency);
private:
protected:
};

} /* namespace ns3 */

}
#endif /* GATEWAY_MESHTASTIC_MAC_H */
