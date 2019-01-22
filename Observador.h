#ifndef OBSERVADOR_H
#define OBSERVADOR_H

// includes de la aplicación
#include "ns3/header.h"
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
#include "ns3/ppp-header.h"
#include <ns3/gnuplot.h>
#include <ns3/queue.h>
#include <ostream>
#include <map>

using namespace ns3;

struct ParametrosObservador {

	Ptr<PointToPointNetDevice> camara;
	Ptr<PointToPointNetDevice> usuario;
	Ptr<PointToPointNetDevice> servidor;

	Ptr<PointToPointNetDevice> enlaceRoutersIzq;
	Ptr<PointToPointNetDevice> enlaceRoutersDer;

};

class Observador : public Object {

	public:
	Observador (ParametrosObservador); // constructor
	// funciones get

	private:

	void Rx(Ptr<const Packet> packet);
	void Tx(Ptr<const Packet> packet);

	Ptr<PointToPointNetDevice> m_camara;
	Ptr<PointToPointNetDevice> m_usuario;
	Ptr<PointToPointNetDevice> m_servidor;

	Ptr<PointToPointNetDevice> enlaceRoutersIzq;
	Ptr<PointToPointNetDevice> enlaceRoutersDer;

	std::map<Ipv4Address, uint32_t> mapaPqtTx;
	std::map<Ipv4Address, uint32_t> mapaPqtRx;




}
