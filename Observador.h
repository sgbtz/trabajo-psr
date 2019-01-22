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


#define TCP 6
#define UDP 17

struct ParametrosObservador {

	Ptr<PointToPointNetDevice> camara;
	Ptr<PointToPointNetDevice> usuario;
	Ptr<PointToPointNetDevice> servidor;
	Ipv4Address ip_cliente;


};

class Observador : public Object {

	public:
	Observador (ParametrosObservador); // constructor

	// funciones get

	private:

/*
	void Rx(Ptr<const Packet> packet);
	void Tx(Ptr<const Packet> packet);

	Ptr<PointToPointNetDevice> m_camara;
	Ptr<PointToPointNetDevice> m_usuario;

	uint32_t m_contadorPqtRx;
	uint32_t m_contadorPqtTx;

	Ipv4Address m_ipCliente;

	//Variables para calcular los estadisticos
	Average<double> m_retardoMedio;
	Average<double> m_variacionRetardo;

	//Mapa para calcular el retardo del streaming
 	std::map<uint32_t, Time> mapaTiempos;
	//Primer retardo para calcular la variacion
	Time m_primerRetardo;
	//Segundo retardo para calcular la variacion
	Time m_segundoRetardo;
	BooleanValue flagFirstDelay;
*/

	// variables parámetro
	Ptr<Application> m_camara;
	Ptr<Application> m_usuario;
	Ptr<Application> m_servidor;

	Ipv4Address m_ipCamara;
	Ipv4Address m_ipUsuario;

	// variables estadísticas Streaming Camara -> Servidor
	uint64_t StreamingServidor_CuentaTx;
	uint64_t StreamingServidor_CuentaRx;
	Average<double> StreamingServidor_PorcentajeCorrectos;
	Average<double> StreamingServidor_Retardo;

	// variables estadísticas Streaming Camara -> Usuario
	uint64_t StreamingUsuario_CuentaTx;
	uint64_t StreamingUsuario_CuentaRx;
	Average<double> StreamingUsuario_PorcentajeCorrectos;
	Average<double> StreamingUsuario_Retardo;

}
