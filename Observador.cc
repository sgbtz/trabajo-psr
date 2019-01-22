#include "Observador.h"
#include "simulacion.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Observador");

struct ParametrosObservador {
	Ptr<Application> camara;
	Ptr<Application> usuario;
	Ptr<Application> servidor;
};

// *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *

// constructor de la clase
Observador::Observador ( ParametrosObservador paramObse ) {	// en el observador se reciben los elementos de interés del escenario
	NS_LOG_FUNCTION("Comienzo");

	// obtener los parámetros e inicializarlos
	m_camara = paramObse.camara;
	m_usuario = paramObse.usuario;
	m_servidor = paramObse.servidor;

	// obtener las IP de interés: usuario y camara
	// ...
	// ...

	// iniciar variables estadísticas
	StreamingServidor_CuentaTx = 0;
	StreamingServidor_CuentaRx = 0;
	StreamingUsuario_CuentaTx = 0;
	StreamingUsuario_CuentaRx = 0;

	// suscribir a trazas
	m_camara->TraceConnectWithoutContext("Tx", MakeCallback (&Observador::CamaraTx, this));
	m_servidor->TraceConnectWithoutContext("Rx", MakeCallback (&Observador::ServidorRx, this));
	m_usuario->TraceConnectWithoutContext("Rx", MakeCallback (&Observador::UsuarioRx, this));

}

// *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *

void Observador::CamaraTx (Ptr<const Packet> paquete) {
	NS_LOG_FUNCTION("Comienzo");

}

// *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *


void Observador::ServidorRx(Ptr<const Packet> paquete, const Address & direccionParam){
	NS_LOG_FUNCTION("Comienzo");

	// leer la dirección origen
	InetSocketAddress miInetSocketAddress = (InetSocketAddress) InetSocketAddress::ConvertFrom(direccionParam);
	Ipv4Address dirOrigen = miInetSocketAddress.GetIpv4();

	// si proviene de la camara observada modificar estadísticos
	if(dirOrigen == m_ipCamara){
		StreamingServidor_CuentaRx ++;
	}

}

// *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *

void Observador::UsuarioRx(Ptr<const Packet> paquete, const Address & direccionParam){
	NS_LOG_FUNCTION("Comienzo");

	// leer la dirección origen
	InetSocketAddress miInetSocketAddress = (InetSocketAddress) InetSocketAddress::ConvertFrom(direccionParam);
	Ipv4Address dirOrigen = miInetSocketAddress.GetIpv4();

	// si proviene de la camara observada modificar estadisticos
	if(dirOrigen == m_ipCamara){
		StreamingUsuario_CuentaRx ++;
	}

}

// *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *

/*	m_contadorPqtRx = 0;
	m_contadorPqtTx = 0;
	flagFirstDelay = true;
*/
	/* Nos subscribimos a las trazas de transmision de paquetes en las camaras, los usuarios y los servidores ya que
		 los 3 envian trafico y queremos analizar el mismo.*/
/*
	m_camara->TraceConnectWithoutContext("MacTx", MakeCallback (&Observador::Tx, this));
	m_usuario->TraceConnectWithoutContext("MacTx", MakeCallback (&Observador::Tx, this));
*/
	/*Nos subscribimos a las trazas de recepcion de paquetes para el analisis del trafico de las camaras, los usuarios
		y los servidores.*/
/*
	m_camara->TraceConnectWithoutContext("MacRx", MakeCallback (&Observador::Rx, this));
	m_usuario->TraceConnectWithoutContext("MacRx", MakeCallback (&Observador::Rx, this));
*/



/*
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

	m_contadorPqtTx;

	//Observamos si se trata de trafico de streaming
	uint8_t protocolo = ip.GetProtocol();

	if ( protocolo == UDP )
	{
		mapaTiempos[packet->GetUid()] = Simulator::Now();
	}

}


void Observador::Rx(Ptr<const Packet > packet){
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
	m_contadorPqtRx;

	//Miramos que protrocolo se usa para saber si se trata de trafico de streaming.
	uint8_t protocolo = ip.GetProtocol();

	if ( protocolo == UDP )
	{
		Time retardo = Simulator::Now() - mapaTiempos[packet->GetUid()];
		NS_LOG_INFO("Retardo del paquete: " << retardo);
		m_retardoMedio.Update(retardo.GetDouble());

		if ( flagFirstDelay){
				m_primerRetardo = retardo;
				flagFirstDelay = false;
		}else{
				m_segundoRetardo = retardo;
				NS_LOG_INFO("Primer retardo: " << m_primerRetardo << " .Segundo retardo: " << m_segundoRetardoUdp);
				NS_LOG_INFO("Variacion del retardo: " << abs((m_segundoRetardo-m_primerRetardo).GetDouble()) << "ns");
				m_variacionRetardo.Update(abs((m_segundoRetardo-m_primerRetardo).GetDouble()));
				m_primerRetardo = m_segundoRetardo;
			}
		}
}
*/
