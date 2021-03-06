/*************** TRABAJO PSR ***************/
/*
**  Simulacion de una empresa de camaras de vigilancia.
**  Fichero de cabecera del archivo principal.
**  Apellidos, Nombres:
**
**    - Cabrera Miñagorri, Miguel Ángel
**    - García Ferrer, Pablo
**    - Guisasola Benítez, Sebastiá
**    - Jiménez Olmedo, Fº Javier
**    - Montero Ramírez, Ezequiel
**
**  Grupo 2
*/

#ifndef SIMULACION_H
#define SIMULACION_H

// includes de la aplicación
#include "ns3/header.h"
#include "ns3/ppp-header.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/application.h"
#include "ns3/stats-module.h"
#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/point-to-point-remote-channel.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/attribute.h"
#include "ns3/onoff-application.h"
#include "ns3/tag.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/stats-module.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/queue-limits.h"
#include "ns3/queue.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/gnuplot.h"
#include "ns3/queue.h"


using namespace ns3;

namespace ns3 {
class PrioPacketFilter : public Ipv4PacketFilter {
public:
  static TypeId GetTypeId (void);

  PrioPacketFilter ();
  ~PrioPacketFilter ();

private:
  int32_t DoClassify (Ptr<QueueDiscItem> item) const;

  uint32_t m_premiumMaxIp;
};
}

// observador

#endif
