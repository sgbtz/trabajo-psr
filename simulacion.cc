/*************** TRABAJO PSR ***************/
/*
**  Simulacion de una empresa de camaras de vigilancia.
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

#define P_ERROR      0.001    // probabilidad de error de todos los enlaces

// parámetros de la transmisión de video
#define MIN_START_VIDEO     "0s"
#define MAX_START_VIDEO     "7h" // 3 robos por semana -> 3/7 = 0.42 -> 1 robo cada 3h/0.42=7h
#define MEAN_DRTN_VIDEO     "15min"

// definiciones generales
#define NUM_USUARIOS        3000 //el numero de usuarios representara tambien el numero de camaras 1 usuario -> 1 camara
#define NUM_SERVIDORES      4
#define CLI_PREMIUM         0.2
#define ENLACES_TRONCALES   2
#define NUM_NODOS_ENLACE    2 //numero de nodos por enlace
// indices para la tablas
#define G1_G3       0
#define G1_G4       1
#define ROUTER      0
#define NODO_FINAL  1

/************** Definición de tipos ***************/

// Estructura para los enlaces del escenario
typedef struct {
  DataRate rate; // capacidad del enlace
  Time delay;    // retardo de propagación
  double perror;
} LinkP2P;

// Estructura para la aplicación de transmisión de video
typedef struct {
  Time minStartVideo; // tiempo mínimo de comienzo
  Time maxStartVideo; // tiempo máximo de comienzo
  Time meanDrtnVideo; // duración media
} VideoApp;

// Estructura para los parametros del escenario
typedef struct {
  LinkP2P serverLink;
  LinkP2P routerLink;
  LinkP2P camLink;
  LinkP2P userLink;
  VideoApp alarmVideo;
  uint32_t numServers;
  uint32_t numUsers;
  double cliPremium; // proporción clientes premium
  double cliBest;    // proporción clientes best-effort
} ParamsEscenario;


/************ Definición de funciones *************/
/*
**  Función encargada de la configuración y simulación del escenario
**
**  - Parámetros de entrada:
**
**  - Parámetros de salida:
**
*/

void escenario(ParamsEscenario paramsEscenario);

int main (int argc, char *argv[]) {
  NS_LOG_FUNCTION("Función main: configuración de los parámetros y generación de gráficas");
  // Establecemos la resolución a nanosegundos y habilitamos checksum
  Time::SetResolution(Time::NS);
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue(true));

  /** Parámetros de los enlaces del modelo **/

  // - grupo de enlaces 1
  LinkP2P serverLink;
  DataRate serverRate (SERVER_RATE);
  Time serverDelay (SERVER_DELAY);
  // - grupo de enlaces 2
  LinkP2P routerLink;
  DataRate routerRate (ROUTER_RATE);
  Time routerDelay (ROUTER_DELAY);
  double routerPerror = P_ERROR;
  // - grupo de enlaces 3
  LinkP2P camLink;
  DataRate camRate (CAM_RATE);
  Time camDelay (CAM_DELAY);
  // - grupo de enlaces 4
  LinkP2P userLink;
  DataRate userRate (USER_RATE);
  Time userDelay (USER_DELAY);

  /** Parámetros de las aplicaciones **/

  // - aplicación para la transmisión de video
  VideoApp alarmVideo;
  Time minStartVideo (MIN_START_VIDEO);
  Time maxStartVideo (MAX_START_VIDEO);
  Time meanDrtnVideo (MEAN_DRTN_VIDEO);


  /** Parámetros de la simulación **/

  uint32_t numServers = NUM_SERVIDORES;
  uint32_t numUsers = NUM_USUARIOS;
  double cliPremium = CLI_PREMIUM;


  /** Añadimos los parametros configurables por linea de comando **/
  CommandLine cmd = CommandLine();
  // Parámetros de la topología
  cmd.AddValue ("tasaServidores", "Capacidad de los enlaces a los servidores", serverRate);
  cmd.AddValue ("tasaRouters", "Capacidad de los enlaces entre routers", routerRate);
  cmd.AddValue ("tasaCamaras", "Capacidad de los enlaces a las camaras", camRate);
  cmd.AddValue ("tasaUsuarios", "Capacidad de los enlaces a los usuarios", userRate);
  cmd.AddValue ("perror", "Probabilidad de error de paquete de los enlaces entre routers", routerPerror);
  cmd.AddValue ("minStartVideo", "Tiempo mínimo de comienzo de transmisión de video", minStartVideo);
  cmd.AddValue ("maxStartVideo", "Tiempo máximo de comienzo de transmisión de video", maxStartVideo);
  cmd.AddValue ("duracionVideo", "Duración media de la transmisión de video", meanDrtnVideo);
  cmd.AddValue ("numServidores", "Número de servidores", numServers);
  cmd.AddValue ("numUsuarios", "Número de usuarios", numUsers);
  cmd.AddValue ("clientesPremium", "Número de servidores", cliPremium);
  cmd.Parse (argc, argv);

  double cliBest = 1 - cliPremium;

  /** Añadimos parámetros al escenario **/
  serverLink.rate = serverRate;
  serverLink.delay = serverDelay;

  routerLink.rate = routerRate;
  routerLink.delay = routerDelay;
  routerLink.perror = routerPerror;

  camLink.rate = serverRate;
  camLink.delay = serverDelay;

  userLink.rate = serverRate;
  userLink.delay = serverDelay;

  alarmVideo.minStartVideo = minStartVideo;
  alarmVideo.maxStartVideo = maxStartVideo;
  alarmVideo.meanDrtnVideo = meanDrtnVideo;

  ParamsEscenario paramsEscenario; // estructura con los parámetros de la simulación
  paramsEscenario.serverLink = serverLink;
  paramsEscenario.routerLink = routerLink;
  paramsEscenario.camLink = camLink;
  paramsEscenario.userLink = userLink;
  paramsEscenario.alarmVideo = alarmVideo;
  paramsEscenario.numServers = numServers;
  paramsEscenario.numUsers = numUsers;
  paramsEscenario.cliPremium = cliPremium;
  paramsEscenario.cliBest = cliBest;

  escenario(paramsEscenario);

  return 0;
}


void escenario(ParamsEscenario paramsEscenario){
  NS_LOG_FUNCTION("Funcion escenario: configuracion basica de un escenario y simulacion");

  /** Configuración del escenario **/
  // Creamos los nodos de cada grupo de enlaces
  NodeContainer grupoUno[paramsEscenario.numServers];
  NodeContainer grupoDos[ENLACES_TRONCALES];
  NodeContainer grupoTres[paramsEscenario.numUsers];
  NodeContainer grupoCuatro[paramsEscenario.numUsers];

  // Enlaces grupo dos (troncales)
  // - enlace servidores - cámaras
  grupoDos[G1_G3].Create(2);
  // - enlace servidores - usuarios
  grupoDos[G1_G4].Add(grupoDos[G1_G3].Get(0));
  grupoDos[G1_G4].Create(1);

  // Enlaces grupo uno (servidores)
  for (uint32_t enlace = 0; enlace < paramsEscenario.numServers; enlace++) {
    grupoUno[enlace].Add(grupoDos[G1_G3].Get(0));
    grupoUno[enlace].Create(NODO_FINAL);
  }
  // Enlaces grupo tres (cámaras) y cuatro (usuarios)
  for (uint32_t enlace = 0; enlace < paramsEscenario.numUsers; enlace++) {
    grupoTres[enlace].Add(grupoDos[G1_G3].Get(1));
    grupoTres[enlace].Create(NODO_FINAL);
    grupoCuatro[enlace].Add(grupoDos[G1_G4].Get(1));
    grupoCuatro[enlace].Create(NODO_FINAL);
  }

  // Configuramos los enlaces punto a punto
  PointToPointHelper enlaceServidores;
  enlaceServidores.SetDeviceAttribute ("DataRate", DataRateValue(paramsEscenario.serverLink.rate));
  enlaceServidores.SetChannelAttribute ("Delay", TimeValue (paramsEscenario.serverLink.delay));

  PointToPointHelper enlaceRouters;
  enlaceRouters.SetDeviceAttribute ("DataRate", DataRateValue(paramsEscenario.routerLink.rate));
  enlaceRouters.SetChannelAttribute ("Delay", TimeValue (paramsEscenario.routerLink.delay));

  PointToPointHelper enlaceCamaras;
  enlaceCamaras.SetDeviceAttribute ("DataRate", DataRateValue(paramsEscenario.camLink.rate));
  enlaceCamaras.SetChannelAttribute ("Delay", TimeValue (paramsEscenario.camLink.delay));

  PointToPointHelper enlaceUsuarios;
  enlaceUsuarios.SetDeviceAttribute ("DataRate", DataRateValue(paramsEscenario.userLink.rate));
  enlaceUsuarios.SetChannelAttribute ("Delay", TimeValue (paramsEscenario.userLink.delay));

  // Establecemos el modelo de errores entre routers
  Ptr<RateErrorModel> remRouter = CreateObject<RateErrorModel> ();
  Ptr<UniformRandomVariable> uvRouter = CreateObject<UniformRandomVariable> ();
  remRouter->SetRandomVariable (uvRouter);
  remRouter->SetUnit(RateErrorModel::ERROR_UNIT_PACKET);
  remRouter->SetRate (paramsEscenario.routerLink.perror);
  enlaceRouters.SetDeviceAttribute("ReceiveErrorModel", PointerValue(remRouter));

  // Creamos un tabla para los NetDeviceContainer
  NetDeviceContainer devicesGrupo1 [paramsEscenario.numServers];
  for (uint32_t n_nodo = 0; n_nodo < paramsEscenario.numServers; n_nodo++) {
    devicesGrupo1[n_nodo] = enlaceServidores.Install(grupoUno[n_nodo]);
  }
  NetDeviceContainer devicesGrupo2 [ENLACES_TRONCALES];
  for (uint32_t n_nodo = 0; n_nodo < ENLACES_TRONCALES; n_nodo++) {
    devicesGrupo2[n_nodo] = enlaceRouters.Install(grupoDos[n_nodo]);
  }
  NetDeviceContainer devicesGrupo3 [paramsEscenario.numUsers];
  NetDeviceContainer devicesGrupo4 [paramsEscenario.numUsers];
  for (uint32_t n_nodo = 0; n_nodo < paramsEscenario.numUsers; n_nodo++) {
    devicesGrupo3[n_nodo] = enlaceCamaras.Install(grupoTres[n_nodo]);
    devicesGrupo4[n_nodo] = enlaceUsuarios.Install(grupoCuatro[n_nodo]);
  }

  /** Configuración IP **/
  // Instalamos la pila TCP/IP en todos los nodos finales
  InternetStackHelper stack;
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numServers; n_nodo++){
    stack.Install(grupoUno[n_nodo].Get(NODO_FINAL)); // instalamos en nodo final
  }
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numUsers; n_nodo++){
    stack.Install(grupoTres[n_nodo].Get(NODO_FINAL)); // instalamos en nodo final
    stack.Install(grupoCuatro[n_nodo].Get(NODO_FINAL)); // instalamos en nodo final
  }
  // Instalamos en los routers
  stack.Install(grupoDos[G1_G3].Get(0));
  stack.Install(grupoDos[G1_G3].Get(1));
  stack.Install(grupoDos[G1_G4].Get(1));

  // Asignamos direcciones IP al grupo1
  Ipv4AddressHelper addressesGrupo1 [paramsEscenario.numServers];
  Ipv4InterfaceContainer interfacesGrupo1 [paramsEscenario.numServers];
  std::string dir;
  char *dir_buff;
  Ipv4Address ipv4Address ("0.0.0.0");
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numServers; n_nodo++){
        dir = ("10.1."+std::to_string(n_nodo+1)+".0");
        NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 1: "<<dir);
        dir_buff = new char[dir.length()]; // dir.length() es el tamano que tendra la ip
        strcpy(dir_buff, dir.c_str());
        ipv4Address.Set(dir_buff);
        addressesGrupo1[n_nodo].SetBase(ipv4Address, "255.255.255.0");
        interfacesGrupo1[n_nodo] = addressesGrupo1[n_nodo].Assign (devicesGrupo1[n_nodo]);
  }
  // Asignamos direcciones al grupo2
  Ipv4AddressHelper addressesGrupo2 [ENLACES_TRONCALES];
  Ipv4InterfaceContainer interfacesGrupo2 [ENLACES_TRONCALES];
  for(uint32_t n_nodo = 0; n_nodo < ENLACES_TRONCALES; n_nodo++){
        dir = ("10.2."+std::to_string(n_nodo+1)+".0");
        NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 2: "<<dir);
        dir_buff = new char[dir.length()];// dir.length() es el tamano que tendra la ip
        strcpy(dir_buff, dir.c_str());
        ipv4Address.Set(dir_buff);
        addressesGrupo2[n_nodo].SetBase(ipv4Address, "255.255.255.0");
        interfacesGrupo2[n_nodo] = addressesGrupo2[n_nodo].Assign (devicesGrupo2[n_nodo]);
  }
  // Asignamos direcciones al grupo3
  Ipv4AddressHelper addressesGrupo3 [paramsEscenario.numUsers];
  Ipv4InterfaceContainer interfacesGrupo3 [paramsEscenario.numUsers];
  Ipv4AddressHelper addressesGrupo4 [paramsEscenario.numUsers];
  Ipv4InterfaceContainer interfacesGrupo4 [paramsEscenario.numUsers];

  uint32_t n_nodo = 0;
  for(uint32_t segundaPosIP = 0; segundaPosIP <= 255 && n_nodo < paramsEscenario.numUsers ; segundaPosIP++){
    for(uint32_t terceraPosIP = 0; terceraPosIP <= 255 && n_nodo < paramsEscenario.numUsers ; terceraPosIP++){
      dir = ("13."+std::to_string(segundaPosIP)+"."+std::to_string(terceraPosIP)+".0");
      NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 3: "<<dir);
      dir_buff = new char[dir.length()];// dir.length() es el tamano que tendra la ip
      strcpy(dir_buff, dir.c_str());
      ipv4Address.Set(dir_buff);
      addressesGrupo3[n_nodo].SetBase(ipv4Address, "255.255.255.0");
      interfacesGrupo3[n_nodo] = addressesGrupo3[n_nodo].Assign (devicesGrupo3[n_nodo]);

      dir = ("14."+std::to_string(segundaPosIP)+"."+std::to_string(terceraPosIP)+".0");
      NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 4: "<<dir);
      dir_buff = new char[dir.length()];// dir.length() es el tamano que tendra la ip
      strcpy(dir_buff, dir.c_str());
      ipv4Address.Set(dir_buff);
      addressesGrupo4[n_nodo].SetBase(ipv4Address, "255.255.255.0");
      interfacesGrupo4[n_nodo] = addressesGrupo4[n_nodo].Assign (devicesGrupo4[n_nodo]);

      // incrementamos el nodo.
      n_nodo++;
    }
  }

  // Poblamos las tablas de rutas
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /** Configuración de las colas de prioridad **/
  // https://www.nsnam.org/docs/models/html/prio.html
  TrafficControlHelper trafficControl;
  // Creamos una cola de prioridad con el mapa de prioridad dado
  uint16_t handle = trafficControl.SetRootQueueDisc ("ns3::PrioQueueDisc", "Priomap", StringValue ("0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1"));
  // Añadimos dos clases de cola
  TrafficControlHelper::ClassIdList classIds = trafficControl.AddQueueDiscClasses (handle, 2, "ns3::QueueDiscClass");
  // Adjuntamos dos colas FIFO para las dos clases añadidas
  trafficControl.AddChildQueueDisc (handle, classIds[0], "ns3::F ifoQueueDisc");
  trafficControl.AddChildQueueDisc (handle, classIds[1], "ns3::FifoQueueDisc");

  /** Configuración de las aplicaciones
  // variables aleatorias con el tiempo de comienzo y fin de la app
  Ptr<RandomVariableStream> alarmVideoStart = CreateObject<UniformRandomVariable>();
  Ptr<RandomVariableStream> alarmVideoStop = CreateObject<UniformRandomVariable>();
  videoStart.SetAttribute("Min", DoubleValue(minStartVideo.GetDouble()))**/
}
//La simulacion hay que pararla manualmente.
