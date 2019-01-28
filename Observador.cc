/*************** TRABAJO PSR ***************/
/*
**  Simulacion de una empresa de camaras de vigilancia.
** 	Fichero de observacion de nodos.
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

#include "Observador.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Observador");

// constructor de la clase
Observador::Observador ( ParametrosObservador parametros ) {	// en el observador se reciben los elementos de interés del escenario
		NS_LOG_FUNCTION("Constructor de observador");
	 	m_camara = parametros.camara;
	 	m_servidor = parametros.servidor;
	 	m_usuario = parametros.usuario;
		m_maxIpServidor = parametros.maxIpServidor;
		m_camaraIp = parametros.ip;

		//iniciamos los contadores de paqeutes.
		pqtTxVidCam2Usr = 0;
		pqtRxVidCam2Usr = 0;
		pqtTxVidCam2Serv = 0;
		pqtRxVidCam2Serv = 0;
		pqtTxInfCam2Serv = 0;
		pqtRxInfCam2Serv = 0;


		flagFirsDelayVidCam2Usr = true;
		flagFirsDelayVidCam2Serv = true;

		varRetMaxVidCam2Serv = 0;
		varRetMaxVidCam2Usr = 0;

		//Nos suscrimos a las trazas de transmision de las camaras
		m_camara->TraceConnectWithoutContext("MacTx", MakeCallback (&Observador::CamaraTx, this));
		//m_servidor->TraceConnectWithoutContext("MacTx", MakeCallback (&Observador::ServidorTx, this));

		//Nos suscribimos a las trazas de recepcion de usuario y servidor.
		m_usuario->TraceConnectWithoutContext("MacRx", MakeCallback (&Observador::UsuarioRx, this));
		m_servidor->TraceConnectWithoutContext("MacRx", MakeCallback (&Observador::ServidorRx, this));
}


void Observador::CamaraTx (Ptr<const Packet> packet) {
	NS_LOG_FUNCTION("Transmision por parte de la camara");

	//Hacemos una copia del paquete original para poder modificarlo.
	Ptr<Packet> pqt = packet->Copy();
	//Obtenemos la cabecera a nivel de enlace y la quitamos
	PppHeader ppp;
	pqt->PeekHeader (ppp);
	pqt->RemoveHeader(ppp);
	//Obtenemos la cabecera a nivel IP para analizar el destino
	Ipv4Header ip;
	pqt->PeekHeader(ip);

	//Guardamos los paquetes en mapas con su timepo dependiendo de si es video o informe.
	uint8_t protocolo = ip.GetProtocol();
	NS_LOG_DEBUG("protocolo camara tx: " << std::to_string(protocolo));
	if ( protocolo == UDP ){
		mapaEnviosCamaraVideo[packet->GetUid()] = Simulator::Now();
		if(ip.GetDestination().Get() >= m_maxIpServidor){
			pqtTxVidCam2Usr++;
			NS_LOG_INFO("paquete enviado de camara a usuario");
		}else{
			pqtTxVidCam2Serv++;
			NS_LOG_INFO("Paquete enviado de camara a servidor");
		}
	}
	if( protocolo == TCP){
		//como siempre los informes van hacia el servidor, sumamos el paquete.
		pqtTxInfCam2Serv++;
		mapaEnviosCamaraInforme[packet->GetUid()] = Simulator::Now();
		NS_LOG_DEBUG("Tamano paquete informe: "<< packet->GetSize());
		NS_LOG_INFO("Paquete enviado de informe de camara a servidor");
	}

}

void Observador::UsuarioRx(Ptr<const Packet > packet){
	NS_LOG_FUNCTION("Paquete recibido por el usuario");

	//Hacemos una copia del paquete original para poder modificarlo.
	Ptr<Packet> pqt = packet->Copy();
	//Obtenemos la cabecera a nivel de enlace y la quitamos
	PppHeader ppp;
	pqt->PeekHeader (ppp);
	pqt->RemoveHeader(ppp);
	//Obtenemos la cabecera a nivel IP para analizar el destino
	Ipv4Header ip;
	pqt->PeekHeader(ip);

	uint8_t protocolo = ip.GetProtocol();
	if(ip.GetSource() == m_camaraIp){
		if ( protocolo == UDP ){
			NS_LOG_INFO("Paquete de video recibido por el usuario");
			//calculamos el retardo.
			double retardo = (Simulator::Now() - mapaEnviosCamaraVideo[packet->GetUid()]).GetMicroSeconds();
			//actualizamos el retardo medio
			retardoVideoUsuario.Update(retardo);
			//como al usuario solo le envia la camara el paquete recibido es de video desde la camara.
			pqtRxVidCam2Usr++;

			//calculamos la variacion del retardo maxima.
			if(flagFirsDelayVidCam2Usr == true){
				varRetMaxVidCam2Usr = 0;
				retardoAntVidCam2Usr = retardo;
				flagFirsDelayVidCam2Usr = false;
			}else{
				double variacion = abs(retardo - retardoAntVidCam2Usr);
				if(variacion > varRetMaxVidCam2Usr)
					varRetMaxVidCam2Usr = variacion;
				//actualizamos el retardo anterior.
				retardoAntVidCam2Usr = retardo;
			}

			//borramos el paqeute del mapa.
			mapaEnviosCamaraVideo.erase(packet->GetUid());
		}
	}
}

void Observador::ServidorRx(Ptr<const Packet > packet){
	NS_LOG_FUNCTION("Paquete recibido por el servidor con uid" << packet->GetUid());
	//Hacemos una copia del paquete original para poder modificarlo.
	Ptr<Packet> pqt = packet->Copy();

	//Obtenemos la cabecera a nivel de enlace y la quitamos
	PppHeader ppp;
	pqt->PeekHeader (ppp);
	pqt->RemoveHeader(ppp);
	//Obtenemos la cabecera a nivel IP para analizar el origen
	Ipv4Header ip;
	pqt->PeekHeader(ip);
	//Miramos que protrocolo se usa para saber si se trata de trafico de streaming.
	uint8_t protocolo = ip.GetProtocol();

	//tenemos en cuenta solo los paquetes que se reciben de la camara que estamos observando.
	if(ip.GetSource() == m_camaraIp){
		if(protocolo == TCP){
			NS_LOG_INFO("Paquete de informe recibido por el servidor");
			//aumentamos la cuenta de paquetes recibidos
			pqtRxInfCam2Serv++;
			double retardo = (Simulator::Now() - mapaEnviosCamaraInforme[packet->GetUid()]).GetMicroSeconds();
			retInfCam2Serv.Update(retardo);
			//sacamos el paqeute del mapa.
			mapaEnviosCamaraInforme.erase(packet->GetUid());
		}
		if(protocolo == UDP){
			NS_LOG_INFO("Paquete de video recibido por el servidor");
			//aumentamos la cuenta de paquetes recibidos
			pqtRxVidCam2Serv++;
			double retardo = (Simulator::Now() - mapaEnviosCamaraVideo[packet->GetUid()]).GetMicroSeconds();
			retVidCam2Serv.Update(retardo);

			//calculamos la variacion de retardo para el video.
			if(flagFirsDelayVidCam2Serv == true){
				varRetMaxVidCam2Serv = 0;
				retardoAntVidCam2Serv = retardo;
				flagFirsDelayVidCam2Serv = false;
			}else{
				double variacion = abs(retardo - retardoAntVidCam2Serv);
				if(variacion > varRetMaxVidCam2Serv)
					varRetMaxVidCam2Serv = variacion;
				//actualizamos el retardo anterior.
				retardoAntVidCam2Serv = retardo;
			}
			//sacamos el paquete del mapa.
			mapaEnviosCamaraVideo.erase(packet->GetUid());
		}
	}
}

void Observador::GetEstadisticos(Average<double> * p_varMaxRetVidCam2Usr, Average<double> * p_retMedVidCam2Usr, Average<double> * p_perdidasVidCam2Usr, Average<double> * p_varMaxRetVidCam2Serv, Average<double> * p_retMedVidCam2Serv, Average<double> * p_perdidasVidCam2Serv, Average<double> * p_retMedInfCam2Serv, Average<double> * p_perdidasInfCam2Serv){
	NS_LOG_FUNCTION("Devolviendo parametros del modelo calculados");

	if(varRetMaxVidCam2Usr > 0){
		p_varMaxRetVidCam2Usr->Update(varRetMaxVidCam2Usr);
		NS_LOG_INFO("Maxima variacion camara a usuario: " << varRetMaxVidCam2Usr);
	}
	if(retardoVideoUsuario.Count() > 0){
		p_retMedVidCam2Usr->Update(retardoVideoUsuario.Max());
		NS_LOG_INFO("Retardo medio video camara a usuario: " << retardoVideoUsuario.Mean());
	}
	if(pqtTxVidCam2Usr > 0) {
		p_perdidasVidCam2Usr->Update((1 - (pqtRxVidCam2Usr / pqtTxVidCam2Usr)) * 100);
		NS_LOG_DEBUG("Paquetes cam2usr " << pqtRxVidCam2Usr << ", " << pqtTxVidCam2Usr);
		NS_LOG_INFO("Porcentaje de paquetes perdidos video camara a usuario: " << ((1 - (pqtRxVidCam2Usr / pqtTxVidCam2Usr)) * 100));
	}
	if(varRetMaxVidCam2Serv > 0){
		p_varMaxRetVidCam2Serv->Update(varRetMaxVidCam2Serv);
		NS_LOG_INFO("Maxima variacion camara a servidor: " << varRetMaxVidCam2Serv);
	}
	if(retVidCam2Serv.Count() > 0){
		p_retMedVidCam2Serv->Update(retVidCam2Serv.Max());
		NS_LOG_INFO("Retardo medio video camara a servidor: " << retVidCam2Serv.Mean());
	}
	if(pqtTxVidCam2Serv > 0){
		p_perdidasVidCam2Serv->Update((1- (pqtRxVidCam2Serv / pqtTxVidCam2Serv)) * 100);
		NS_LOG_DEBUG("Paquetes cam2serv " << pqtRxVidCam2Serv << ", " << pqtTxVidCam2Serv);
		NS_LOG_INFO("Porcentaje de paquetes perdidos video camara a servidor: " << ((1- (pqtRxVidCam2Serv / pqtTxVidCam2Serv)) * 100));
	}
	if(retInfCam2Serv.Count() > 0){
		p_retMedInfCam2Serv->Update(retInfCam2Serv.Max());
		NS_LOG_INFO("Retardo medio informes camara a servidor: " << retInfCam2Serv.Mean());
	}
	if(pqtTxInfCam2Serv > 0){
		p_perdidasInfCam2Serv->Update((1- (pqtRxInfCam2Serv / pqtTxInfCam2Serv)) * 100);
		NS_LOG_DEBUG("Paquetes informe " << pqtRxInfCam2Serv << ", " << pqtTxInfCam2Serv);
		NS_LOG_INFO("Porcentaje de paquetes perdidos informes camara a servidor: " << ((1- (pqtRxInfCam2Serv / pqtTxInfCam2Serv)) * 100));
	}
}
