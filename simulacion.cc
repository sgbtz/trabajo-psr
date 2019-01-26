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
#include "Observador.h"

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
#define MAX_START_VIDEO     "7h" // 3 alarms por semana -> 3/7 = 0.42 -> 1 alarm cada 3h/0.42=7h
#define MEAN_DRTN_VIDEO     "15min"

// parámetros del envío de informes en bits
#define TAM_MEDIO_INFO      200000000

// definiciones generales
#define NUM_USUARIOS        3000 //el numero de usuarios representara tambien el numero de camaras 1 usuario -> 1 camara
#define NUM_SERVIDORES      4
#define NUM_PREMIUM         2500
#define ENLACES_TRONCALES   2
#define NUM_NODOS_ENLACE    2 //numero de nodos por enlace
#define T_START             "0s"
#define T_STOP              "50s"
// indices para la tablas
#define G1_G3       0
#define G1_G4       1
#define ROUTER      0
#define NODO_FINAL  1

//definimos los identificadores de protocolo de TCP y UDP
#define PROTOCOLO_TCP 6
#define PROTOCOLO_UDP 17
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

// Estructura para la aplicación del envío de informes
typedef struct {
  double tamMedioInforme;
} InfoApp;

// Estructura para los parametros del escenario
typedef struct {
  LinkP2P serverLink;
  LinkP2P routerLink;
  LinkP2P camLink;
  LinkP2P userLink;
  VideoApp alarmVideo;
  InfoApp infoReport;
  uint32_t numServers;
  uint32_t numUsers;
  uint32_t numPremium;    // numero de clientes premium
  Time tstart;            // comienzo simulación
  Time tstop;             // fin simulación
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

  // - aplicación para el envío de informes
  InfoApp infoReport;
  double tamMedioInforme = TAM_MEDIO_INFO;


  /** Parámetros de la simulación **/

  uint32_t numServers = NUM_SERVIDORES;
  uint32_t numUsers = NUM_USUARIOS;
  uint32_t numPremium = NUM_PREMIUM;
  Time tstart(T_START);
  Time tstop(T_STOP);


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
  cmd.AddValue ("tamInformes", "Tamaño medio de los informes", tamMedioInforme);
  cmd.AddValue ("numServidores", "Número de servidores", numServers);
  cmd.AddValue ("numUsuarios", "Número de usuarios", numUsers);
  cmd.AddValue ("clientesPremium", "numero de usuarios premium", numPremium);
  cmd.AddValue ("startTime", "momento final de la simulacion", tstart);
  cmd.AddValue ("stopTime", "momento final de la simulacion", tstop);

  cmd.Parse (argc, argv);

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

  infoReport.tamMedioInforme = tamMedioInforme;

  ParamsEscenario paramsEscenario; // estructura con los parámetros de la simulación
  paramsEscenario.serverLink = serverLink;
  paramsEscenario.routerLink = routerLink;
  paramsEscenario.camLink = camLink;
  paramsEscenario.userLink = userLink;
  paramsEscenario.alarmVideo = alarmVideo;
  paramsEscenario.infoReport = infoReport;
  paramsEscenario.numServers = numServers;
  paramsEscenario.numUsers = numUsers;
  paramsEscenario.numPremium = numPremium;
  paramsEscenario.tstart = tstart;
  paramsEscenario.tstop = tstop;


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

  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numServers; n_nodo++){
        std::string dir = ("10.1."+std::to_string(n_nodo+1)+".0");
        NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 1: "<<dir);
        char *dir_buff = new char[dir.length()]; // dir.length() es el tamano que tendra la ip
        strcpy(dir_buff, dir.c_str());
        Ipv4Address ipv4Address ("0.0.0.0");
        ipv4Address.Set(dir_buff);
        addressesGrupo1[n_nodo].SetBase(ipv4Address, "255.255.255.0");
        interfacesGrupo1[n_nodo] = addressesGrupo1[n_nodo].Assign (devicesGrupo1[n_nodo]);
  }
  // Asignamos direcciones al grupo2
  Ipv4AddressHelper addressesGrupo2 [ENLACES_TRONCALES];
  Ipv4InterfaceContainer interfacesGrupo2 [ENLACES_TRONCALES];
  for(uint32_t n_nodo = 0; n_nodo < ENLACES_TRONCALES; n_nodo++){
        std::string dir = ("10.2."+std::to_string(n_nodo+1)+".0");
        NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 2: "<<dir);
        char *dir_buff = new char[dir.length()];// dir.length() es el tamano que tendra la ip
        strcpy(dir_buff, dir.c_str());
        Ipv4Address ipv4Address ("0.0.0.0");
        ipv4Address.Set(dir_buff);
        addressesGrupo2[n_nodo].SetBase(ipv4Address, "255.255.255.0");
        interfacesGrupo2[n_nodo] = addressesGrupo2[n_nodo].Assign (devicesGrupo2[n_nodo]);
  }
  // Asignamos direcciones al grupo3 y grupo4
  Ipv4AddressHelper addressesGrupo3 [paramsEscenario.numUsers];
  Ipv4InterfaceContainer interfacesGrupo3 [paramsEscenario.numUsers];
  Ipv4AddressHelper addressesGrupo4 [paramsEscenario.numUsers];
  Ipv4InterfaceContainer interfacesGrupo4 [paramsEscenario.numUsers];

  uint32_t n_nodo = 0;
  uint32_t premiumMaxIp;//para almacenar la ip del ultimo nodo premium
  for(uint32_t segundaPosIP = 0; segundaPosIP <= 255 && n_nodo < paramsEscenario.numUsers ; segundaPosIP++){
    for(uint32_t terceraPosIP = 0; terceraPosIP <= 255 && n_nodo < paramsEscenario.numUsers ; terceraPosIP++){
      std::string dir = ("13."+std::to_string(segundaPosIP)+"."+std::to_string(terceraPosIP)+".0");
      NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 3: "<<dir);
      char *dir_buff = new char[dir.length()];// dir.length() es el tamano que tendra la ip
      strcpy(dir_buff, dir.c_str());
      Ipv4Address ipv4Address ("0.0.0.0");
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
      // obtenemos la direccion del ultimo nodo premium para clasificar en las colas.
      if (n_nodo == paramsEscenario.numPremium-1)
        premiumMaxIp = ipv4Address.Get();
    }
  }

  /** Configuración de la prioridad en las colas **/
  TrafficControlHelper trafficControl;
  // Creamos una cola de prioridad con el mapa de prioridad dado
  uint16_t handle = trafficControl.SetRootQueueDisc ("ns3::PrioQueueDisc", "Priomap", StringValue ("0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3"));
  // Añadimos dos clases de cola
  TrafficControlHelper::ClassIdList classIds = trafficControl.AddQueueDiscClasses (handle, 4, "ns3::QueueDiscClass");
  // Adjuntamos dos colas FIFO para las dos clases añadidas
  trafficControl.AddChildQueueDisc (handle, classIds[0], "ns3::FifoQueueDisc"); // para el video de premium
  trafficControl.AddChildQueueDisc (handle, classIds[1], "ns3::FifoQueueDisc"); // para el video de no premium
  trafficControl.AddChildQueueDisc (handle, classIds[2], "ns3::FifoQueueDisc"); // para el informe de premium
  trafficControl.AddChildQueueDisc (handle, classIds[3], "ns3::FifoQueueDisc"); // para el informe de no premium

  // Instalamos el controlador de tráfico en cada router
  NetDeviceContainer routerDevices;
  routerDevices.Add(devicesGrupo2[G1_G3].Get(0));
  routerDevices.Add(devicesGrupo2[G1_G3].Get(1));
  routerDevices.Add(devicesGrupo2[G1_G4].Get(1));
  trafficControl.Uninstall(routerDevices);
  trafficControl.Install(routerDevices);
  // Añadimos un filtro de paquetes para encolar por prioridad
  trafficControl.AddPacketFilter (handle, "ns3::PrioPacketFilter", "PremiumMaxIp", UintegerValue(premiumMaxIp));
  ////FIN CONFIGURACION COLAS////

  NS_LOG_INFO("Maxima direccion premium: " << premiumMaxIp);
  // Poblamos las tablas de rutas
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* ______________________________________*/
  /* Separacion entre premium y no premium */
  /**-------------------------------------**/
  NodeContainer camarasPremium;
  NodeContainer camarasNoPremium;
  NodeContainer usuarioPremium;
  NodeContainer usuarioNoPremium;
  for(uint32_t usuario=0; usuario<paramsEscenario.numUsers; usuario++){
    // los numPremium primeros usuarios seran premium, el resto best-effort
    if(usuario < paramsEscenario.numPremium){
      camarasPremium.Add(grupoTres[usuario].Get(NODO_FINAL));
      usuarioPremium.Add(grupoCuatro[usuario].Get(NODO_FINAL));
    }else{
      camarasNoPremium.Add(grupoTres[usuario].Get(NODO_FINAL));
      usuarioNoPremium.Add(grupoCuatro[usuario].Get(NODO_FINAL));
    }
  }

  /*__________________________________________**
  ** Creacion de aplicaciones y configuracion **
  **------------------------------------------**/
  // creamos helpers para crear los sumideros de video y de informes.
  uint16_t videoPort = 9;
  uint16_t informePort = 10;
  PacketSinkHelper receptorVideoHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), videoPort)));
  PacketSinkHelper receptorInformeHelper ("ns3::TcpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), informePort)));

  // instalamos los sumideros en los servidores
  ApplicationContainer receptorVideoServidor[paramsEscenario.numServers];
  ApplicationContainer receptorInformeServidor[paramsEscenario.numServers];
  for (uint32_t n_server = 0; n_server < paramsEscenario.numServers; n_server++) {
    receptorVideoServidor[n_server] = receptorVideoHelper.Install(grupoUno[n_server].Get(NODO_FINAL));
    receptorVideoServidor[n_server].Start(paramsEscenario.tstart);
    receptorInformeServidor[n_server] = receptorInformeHelper.Install(grupoUno[n_server].Get(NODO_FINAL));
    receptorInformeServidor[n_server].Start(paramsEscenario.tstart);
  }

  // instalamos los sumideros en los usuarios.
  ApplicationContainer receptorVideoUsuario[paramsEscenario.numUsers];
  ApplicationContainer receptorInformeUsuario[paramsEscenario.numUsers];
  for (uint32_t n_user = 0; n_user < paramsEscenario.numUsers; n_user++) {
    receptorVideoUsuario[n_user] = receptorVideoHelper.Install(grupoCuatro[n_user].Get(NODO_FINAL));
    receptorVideoUsuario[n_user].Start(paramsEscenario.tstart);
    receptorInformeUsuario[n_user] = receptorInformeHelper.Install(grupoCuatro[n_user].Get(NODO_FINAL));
    receptorInformeUsuario[n_user].Start(paramsEscenario.tstart);
  }


  /*emisores de video hacia los usuarios desde los servidores. Tendremos un container por app
  ApplicationContainer appVideoServidor2Usuarios[paramsEscenario.numUsers];*/
  // Aplicaciones de transmisión
  ApplicationContainer appInformeCam2Server[paramsEscenario.numUsers]; // envío periódico de informe de la cámara al servidor
  ApplicationContainer appInformeServer2User[paramsEscenario.numUsers]; // envío de informe del servidor al usuario
  ApplicationContainer appAlarmVideoCam2Server[paramsEscenario.numUsers]; // streaming de vídeo por alarma al servidor
  ApplicationContainer appAlarmVideoCam2User[paramsEscenario.numUsers]; // streaming de vídeo por alarma al usuario

  //Array qe contiene el numero de servidor al que esta conectado el usuario indice de la tabla.
  uint32_t connServUser[paramsEscenario.numUsers];

  // configuramos e instalamos las aplicaciones en las camaras y los servidores, las camaras y usuarios respectivos estaran
  // conectadas al mismo servidor, ademas, para simular el streaming se iniciaran a la VEZ <----- Esto se va a cambiar ???
  // la transmision desde la camara al servidor y desde el servidor al usuario.
  for (uint32_t n_nodo = 0; n_nodo < paramsEscenario.numUsers; n_nodo++) {
    // v.a. para modelar el momento y duracion de la alarma (robo o falsa)
    Ptr<UniformRandomVariable> alarmStart = CreateObject<UniformRandomVariable>();
    Ptr<ExponentialRandomVariable> alarmTime = CreateObject<ExponentialRandomVariable>();
    alarmTime->SetAttribute("Mean", DoubleValue(paramsEscenario.alarmVideo.meanDrtnVideo.GetSeconds()));
    //v.a. para modelar el tamano y momento de envio del informes
    Ptr<UniformRandomVariable> momentoEnvioInforme = CreateObject<UniformRandomVariable>();
    Ptr<ExponentialRandomVariable> informeSize = CreateObject<ExponentialRandomVariable>();
    informeSize->SetAttribute("Mean", DoubleValue(paramsEscenario.infoReport.tamMedioInforme));

    // obtenemos y guardamos los valores de incio de las alarmas y su duracion.
    Time inicioAlarma (alarmStart->GetValue(paramsEscenario.alarmVideo.minStartVideo.GetDouble(), paramsEscenario.alarmVideo.maxStartVideo.GetDouble()));
    Time duracionAlarma (alarmTime->GetValue());
    // guardamos los valores de incio de transmision y de tamano de los informe
    uint64_t tamInformes = (uint64_t) informeSize->GetValue();
    NS_LOG_DEBUG("tamano de los informes: " << tamInformes);
    // los informes se solicitan tras el final de la alarma
    Time inicioTxInforme (inicioAlarma.GetDouble() + duracionAlarma.GetDouble());

    // para simular que los usuarios estan repartidos entre los servidores usaremos una variable aleatoria que determinara el servidor al que se conecta el usuario.
    Ptr<UniformRandomVariable> conectarCon = CreateObject<UniformRandomVariable>();
    uint32_t conServer = (uint32_t) conectarCon->GetValue(0, paramsEscenario.numServers-1); //tanto la camara como la aplicacion del usuario conectaran con el mismo servidor, determinado por esta variable.
    //guardamos el servidor al que se ha conectado el usuario.
    connServUser[n_nodo] = conServer;
    // creamos los helpers para las emisiones de contenido.
    UdpClientHelper emisorAlarmVideoCam2User = UdpClientHelper(Address (InetSocketAddress (interfacesGrupo4[n_nodo].GetAddress(NODO_FINAL), videoPort)));
    UdpClientHelper emisorAlarmVideoCam2Server = UdpClientHelper(Address (InetSocketAddress (interfacesGrupo1[conServer].GetAddress(NODO_FINAL), videoPort)));
    // BulkSendHelper emisorInformeServer2User = BulkSendHelper("ns3::TcpSocketFactory", Address (InetSocketAddress (interfacesGrupo4[n_nodo].GetAddress(NODO_FINAL), informePort)));
    BulkSendHelper emisorInformeCam2Server = BulkSendHelper("ns3::TcpSocketFactory", Address (InetSocketAddress (interfacesGrupo1[conServer].GetAddress(NODO_FINAL), informePort)));

    // configuramos con las v.a. el emisor de video en streaming cuando hay alarma.
    emisorAlarmVideoCam2User.SetAttribute("StartTime", TimeValue(inicioAlarma));
    emisorAlarmVideoCam2User.SetAttribute("StopTime", TimeValue(inicioAlarma + duracionAlarma));
    emisorAlarmVideoCam2Server.SetAttribute("StartTime", TimeValue(inicioAlarma));
    emisorAlarmVideoCam2Server.SetAttribute("StopTime", TimeValue(inicioAlarma + duracionAlarma));
    // configuramos los helpers para emision de informes.
    /* la peticion de transmision de informe del servidor al usuario viene dada por unos patrones de tiempo (por determinar)
    emisorInformeServer2User.SetAttribute("StartTime", TimeValue(inicioTxInforme));
    emisorInformeServer2User.SetAttribute("SendSize", UintegerValue(tamInformes));*/
    emisorInformeCam2Server.SetAttribute("StartTime", TimeValue(inicioTxInforme));
    emisorInformeCam2Server.SetAttribute("SendSize", UintegerValue(tamInformes));
    emisorInformeCam2Server.SetAttribute("MaxBytes", UintegerValue(tamInformes));
    

    // realizamos las instalaciones.
    appAlarmVideoCam2Server[n_nodo] = emisorAlarmVideoCam2Server.Install(grupoTres[n_nodo]);
    appAlarmVideoCam2User[n_nodo] = emisorAlarmVideoCam2User.Install(grupoTres[n_nodo]);
    appInformeCam2Server[n_nodo] = emisorInformeCam2Server.Install(grupoTres[n_nodo]);
    // appInformeServer2User[n_nodo] = emisorInformeServer2User.Install(grupoUno[conServer]);
  }

  /** Creación de observadores **/
  //observadores para los usuarios premium.
  Observador * obsPremium[paramsEscenario.numPremium];
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numPremium; n_nodo++){
    ParametrosObservador params;
    params.camara = devicesGrupo3[n_nodo].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.usuario = devicesGrupo4[n_nodo].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.servidor = devicesGrupo1[connServUser[n_nodo]].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.maxIpServidor = premiumMaxIp;
    obsPremium[n_nodo] = new Observador(params);
  }
  //observadores para los usuarios no premium.
  Observador * obsNoPremium[paramsEscenario.numUsers - paramsEscenario.numPremium];
  for(uint32_t n_nodo = paramsEscenario.numPremium; n_nodo < paramsEscenario.numUsers; n_nodo++){
    ParametrosObservador params;
    params.camara = devicesGrupo3[n_nodo].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.usuario = devicesGrupo4[n_nodo].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.servidor = devicesGrupo1[connServUser[n_nodo]].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.maxIpServidor = premiumMaxIp;
    obsNoPremium[n_nodo - paramsEscenario.numPremium] = new Observador(params);
  }

  //paramos la simulacion en el tstop ya que los eventos simulados pueden haberse configurado tras las 3h analizadas,
  //puesto que hemos adaptado la probabilidad de que salte una alarma.
  Simulator::Stop(paramsEscenario.tstop);
  Simulator::Run();

  NS_LOG_DEBUG("observador premium cero: " << obsNoPremium[0]);
  NS_LOG_DEBUG("observador no premium cero: " << obsPremium[0]);
  double varMaxRetVidCam2Usr;
  double retMedVidCam2Usr;
  double perdidasVidCam2Usr;
  double varMaxRetVidCam2Serv;
  double retMedVidCam2Serv;
  double perdidasVidCam2Serv;
  double retMedInfCam2Serv;
  double perdidasInfCam2Serv;
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numPremium; n_nodo++){
    obsPremium[n_nodo]->GetEstadisticos(varMaxRetVidCam2Usr,retMedVidCam2Usr,perdidasVidCam2Usr,varMaxRetVidCam2Serv,retMedVidCam2Serv,perdidasVidCam2Serv,retMedInfCam2Serv,perdidasInfCam2Serv);
  }
//IMPORTANTE:  para los calculos medios de los observadores y las graficas hay
//que tener en cuenta que cuando la alarma no salta los valores son 0, por tanto
//si son 0 no deben anadirse a las medias.

  Simulator::Destroy();

  //destruimos los objetos observadores.
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numPremium; n_nodo++){
    delete obsPremium[n_nodo];
  }
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numUsers - paramsEscenario.numPremium; n_nodo++){
    delete obsNoPremium[n_nodo];
  }

}


/*-----------------------------------------------------------------------*/
/** DEFINICION DE FUNCIONES PARA IMPLEMETAR EL CLASIFICADOR DE PAQUETES **/
/*-----------------------------------------------------------------------*/
NS_OBJECT_ENSURE_REGISTERED (PrioPacketFilter);
TypeId
PrioPacketFilter::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PrioPacketFilter")
    .SetParent<Ipv4PacketFilter> ()
    .SetGroupName ("Internet")
    .AddConstructor<PrioPacketFilter> ()
    .AddAttribute ("PremiumMaxIp",
                   "Mayor direccion IP de usuario premium.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&PrioPacketFilter::m_premiumMaxIp),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

PrioPacketFilter::PrioPacketFilter ()
{
  NS_LOG_FUNCTION (this);
}

PrioPacketFilter::~PrioPacketFilter ()
{
  NS_LOG_FUNCTION (this);
}

int32_t
PrioPacketFilter::DoClassify (Ptr<QueueDiscItem> item) const
{
  NS_LOG_FUNCTION("Classifying packet");
  Ptr<Packet> packet = item->GetPacket();

  Ptr<Packet> tmpPacket = packet->Copy();
  PppHeader ppp;
  tmpPacket->PeekHeader(ppp);
  tmpPacket->RemoveHeader(ppp);
  // Cabecera ip del paquete
  Ipv4Header ipHeader;
  tmpPacket->PeekHeader(ipHeader);
  // Cola en la que se encolará el paquete
  int32_t cola = 0;
  //ip origen del paquete
  Ipv4Address addr = ipHeader.GetSource();
  NS_LOG_INFO(addr);
  uint32_t addr32bit = addr.Get();

  uint8_t protocolo = ipHeader.GetProtocol();
  if (addr32bit < m_premiumMaxIp) { // si es premium
    //miramos el protocolo para diferenciar entre video e informes
    if (protocolo == PROTOCOLO_TCP) // si informes
      cola = 2;
    else if (protocolo == PROTOCOLO_UDP) // si video
      cola = 0;
  } else { //no premium
    if (protocolo == PROTOCOLO_TCP)
      cola = 3;
    else if (protocolo == PROTOCOLO_UDP)
      cola = 1;
  }

  return cola;
}
