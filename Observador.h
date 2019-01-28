#ifndef OBSERVADOR_H
#define OBSERVADOR_H

// includes de la aplicación
#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/callback.h"
#include "ns3/csma-module.h"
#include "ns3/average.h"
#include "ns3/net-device.h"
#include "ns3/applications-module.h"
#include "ns3/socket.h"
#include "ns3/inet-socket-address.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/ppp-header.h"
#include "ns3/sequence-number.h"
#include "ns3/tcp-header.h"
#include "ns3/ipv4-header.h"

using namespace ns3;


#define TCP 6
#define UDP 17

typedef struct{
	Ptr<PointToPointNetDevice> camara;
	Ptr<PointToPointNetDevice> usuario;
	Ptr<PointToPointNetDevice> servidor;
	uint32_t maxIpServidor;
}ParametrosObservador;

class Observador : public Object {

	public:
		Observador (ParametrosObservador parametros); // constructor
		void GetEstadisticos(Average<double> * p_varMaxRetVidCam2Usr, Average<double> * p_retMedVidCam2Usr, Average<double> * p_perdidasVidCam2Usr, Average<double> * p_varMaxRetVidCam2Serv, Average<double> * p_retMedVidCam2Serv, Average<double> * p_perdidasVidCam2Serv, Average<double> * p_retMedInfCam2Serv, Average<double> * p_perdidasInfCam2Serv);

	private:

		void UsuarioRx(Ptr<const Packet> packet);
		void CamaraTx(Ptr<const Packet> packet);
		void ServidorRx(Ptr<const Packet> packet);
		//void ServidorTx(Ptr<const Packet> packet);

		Ptr<PointToPointNetDevice> m_camara;
		Ptr<PointToPointNetDevice> m_usuario;
		Ptr<PointToPointNetDevice> m_servidor;

		//mapa para almacenar las transmisiones de las camaras.
	 	std::map<uint32_t, Time> mapaEnviosCamaraVideo;
		std::map<uint32_t, Time> mapaEnviosCamaraInforme;

		uint32_t m_maxIpServidor;

		//variables para calculos entre camara y usuario.
		Average<double> retardoVideoUsuario;
		double retardoAntVidCam2Usr;
		double varRetMaxVidCam2Usr;
		BooleanValue flagFirsDelayVidCam2Usr;
		uint32_t pqtTxVidCam2Usr;
		uint32_t pqtRxVidCam2Usr;

		//variables para trafico entre camara y servidor.
		uint32_t pqtTxVidCam2Serv;
		uint32_t pqtRxVidCam2Serv;
		uint32_t pqtTxInfCam2Serv;
		uint32_t pqtRxInfCam2Serv;
		Average<double> retVidCam2Serv;
		Average<double> retInfCam2Serv;
		double retardoAntVidCam2Serv;
		double varRetMaxVidCam2Serv;
		BooleanValue flagFirsDelayVidCam2Serv;



};

#endif
