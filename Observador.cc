#include "Observador.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Observador");

// constructor de la clase
Observador::Observador ( ParametrosObservador paramObse ) {	// en el observador se reciben los elementos de interés del escenario

	NS_LOG_FUNCTION("Comienzo");

	// obtener los parámetros
	m_camara = paramObse.camara;
	m_usuario = paramObse.usuario;
	m_servidor = paramObse.servidor;

	enlaceRoutersIzq = paramObse.enlaceRoutersIzq;
	enlaceRoutersDer = paramObse.enlaceRoutersDer;

	/* Nos subscribimos a las trazas de transmision de paquetes en las camaras, los usuarios y los servidores ya que
		 los 3 envian trafico y queremos analizar el mismo.*/

	m_camara->TraceConnectWithoutContext("MacTx", MakeCallback (&Observador::Tx, this));
	m_usuario->TraceConnectWithoutContext("MacTx", MakeCallback (&Observador::Tx, this));
	m_servidor->TraceConnectWithoutContext("MacTx", MakeCallback (&Observador::Tx, this));

	/*Nos subscribimos a las trazas de recepcion de paquetes para el analisis del trafico de las camaras, los usuarios
		y los servidores.*/
	m_camara->TraceConnectWithoutContext("MacRx", MakeCallback (&Observador::Rx, this));
	m_usuario->TraceConnectWithoutContext("MacRx", MakeCallback (&Observador::Rx, this));
	m_servidor->TraceConnectWithoutContext("MacRx", MakeCallback (&Observador::Rx, this));
}


void Observador::Rx (Ptr<const Packet > packet){
	NS_LOG_FUNCTION("Paquete recibido");

	//Hacemos una copia del paquete original para poder modificarlo.
	Ptr<Packet> pqt = packet->Copy();

	//Obtenemos la cabecera a nivel de enlace y la quitamos
	PppHeader ppp;
	pqt->PeekHeader (ppp);
	pqt->RemoveHeader(ppp);
	//Obtenemos la cabecera a nivel IP para analizar el origen

	Ipv4Header ip;
	pqt->PeekHeader(ip);

	//Aniadimos un paquete Rx del origen para el analisis de paquetes perdidos.
	mapaPqtRx[ip.GetSource()] +=1;

}


void Observador::Tx (Ptr<const Packet> packet) {
	NS_LOG_FUNCTION("Paquete transmitido");

	//Hacemos una copia del paquete original para poder modificarlo.
	Ptr<Packet> pqt = packet->Copy();

	//Obtenemos la cabecera a nivel de enlace y la quitamos
	PppHeader ppp;
	pqt->PeekHeader (ppp);
	pqt->RemoveHeader(ppp);
	//Obtenemos la cabecera a nivel IP para analizar el origen

	Ipv4Header ip;
	pqt->PeekHeader(ip);

	mapaPqtTx[ip.GetSource()] +=1;

}
