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
#include "Observador.h"

using namespace ns3;

/************* Definición de macros **************/
#define ROUTER_RATE  "10Mb/s"
#define SERVER_RATE  "5Mb/s"
#define CAM_RATE     "3Mb/s"
#define USER_RATE    "1Mb/s"
#define ROUTER_DELAY "10ms"
#define SERVER_DELAY "1ms"
#define CAM_DELAY    "2ms"
#define USER_DELAY   "3ms"
#define PERROR 0.01
#define T_ON "1s"
#define T_OFF "0s"
#define T_START "1s"
#define T_STOP  "1h"
#define ON_RATE "500kb/s"
#define PCKT_LEN 1220
#define TSTUDENT 2.2622
#define MUESTRAS 10
#define UDP 0
#define TCP 1

/************** Definición de tipos ***************/
// Estructura para los enlaces del esecenario
typedef struct {
  DataRate rate; // capacidad del enlace
  Time delay; // Retardo de propagación del enlace
} LinkP2P;

NS_LOG_COMPONENT_DEFINE ("simulacion");

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
  // - del enlace punto a punto de los encaminadores
  LinkP2P routerLink;
  routerLink.rate(ROUTER_RATE);  // capacidad del enlace entre encaminadores
  routerLink.delay(ROUTER_DELAY); // retardo de propagación del enlace entre encaminadores
  // - de los enlaces punto a punto a los servidores
  LinkP2P serverLink;
  serverLink.rate(SERVER_RATE);  // capacidad del enlace entre encaminadores
  serverLink.delay(SERVER_DELAY); // retardo de propagación del enlace entre encaminadores
  // - de los enlaces punto a punto a las camaras
  LinkP2P camLink;
  camLink.rate(CAM_RATE);  // capacidad del enlace entre encaminadores
  camLink.delay(CAM_DELAY); // retardo de propagación del enlace entre encaminadores
  // - de los enlaces punto a punto a los usuarios
  LinkP2P userLink;
  userLink.rate(USER_RATE);  // capacidad del enlace entre encaminadores
  userLink.delay(USER_DELAY); // retardo de propagación del enlace entre encaminadores




  return 0;
}
