#include "Observador.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Observador");

// constructor de la clase
Observador::Observador ( ParametrosObservador paramObse ) {	// en el observador se reciben los elementos de interés del escenario

	NS_LOG_FUNCTION("comienzo");

	// obtener los parámetros
	camaras = paramObse.camaras;
	usuarios = paramObse.usuarios;
	servidores = paramObse.servidores;

	enlaceRoutersIzq = paramObse.enlaceRoutersIzq; 
	enlaceRoutersDer = paramObse.enlaceRoutersDer;
	
}