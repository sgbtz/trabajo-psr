/*************** TRABAJO PSR ***************/
/*
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

#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/core-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/gnuplot.h"
#include "Simulacion.h" // fichero de cabecera 

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("simulacion");

/* 
** EN ESTE CÓDIGO SE VAN A CREAR LOS ENLACES P2P NECESARIOS PARA CONSTRUIR ESTE ESCENARIO
**
**
**                        servidor    servidor    servidor
**                                \      |       / 
**  grupo enlaces 1 -->            \     |      /
**                                  \    |     /
**                                    routerSer
**                                       /  \
**  grupo enlaces 2 -->                 /    \
**                                     /      \
**                               routerCam     routerUsr
**                               /  |  \         /  |  \
**  grupo enlaces 3 -->         /   |   \       /   |   \        <-- grupo enlaces 4
**                             /    |    \     /    |    \
**                           cam   cam   cam  usr  usr   usr
**
**
** UNA VEZ TENGAMOS LOS NODOS SE CREAN LAS APLICACIONES ONOFF UDP/TCP SOBRE ELLOS
**
*/

/************* Definición de macros **************/

// rate de cada grupo
#define SERVER_RATE  "5Mb/s"	// g1
#define ROUTER_RATE  "10Mb/s"	// g2
#define CAM_RATE     "3Mb/s"	// g3
#define USER_RATE    "1Mb/s"	// g4

// delay de cada grupo
#define SERVER_DELAY "1ms"		// g1
#define ROUTER_DELAY "10ms"		// g2
#define CAM_DELAY    "2ms"		// g3
#define USER_DELAY   "3ms"		// g4


/************** Definición de tipos ***************/

// Estructura para los enlaces del escenario
typedef struct {
  DataRate rate; 	// capacidad del enlace
  Time delay; 		// Retardo de propagación del enlace
} LinkP2P;


/************ Definición de funciones *************/
/*
**  Función encargada de la configuración y simulación del escenario
**
**  - Parámetros de entrada:
**
**  - Parámetros de salida:
**
*/

void escenario(LinkP2P routerLink, LinkP2P serverLink, LinkP2P camLink, LinkP2P userLink);

int main (int argc, char *argv[]) {
  // Establecemos la resolución a nanosegundos y habilitamos checksum
  Time::SetResolution(Time::NS);
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue(true));

  // Parámetros del modelo

  // grupo 1
  DataRate serverRate (SERVER_RATE);
  Time serverDelay (SERVER_DELAY);

  // grupo 2
  DataRate routerRate (ROUTER_RATE);
  Time routerDelay (ROUTER_DELAY);  

  // grupo 3
  DataRate camRate (CAM_RATE);
  Time camDelay (CAM_DELAY);

  // grupo 4
  DataRate usrRate (USER_RATE);
  Time usrDelay (USER_DELAY);

  return 0;
}
