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
*******************************************/

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
#define SERVER_RATE  "500kb/s"	// g1
#define ROUTER_RATE  "5Mb/s"	// g2
#define CAM_RATE     "500kb/s"	// g3
#define USER_RATE    "500kb/s"	// g4

// delay de cada grupo
#define SERVER_DELAY "2ms"		// g1
#define ROUTER_DELAY "1ms"		// g2
#define CAM_DELAY    "3ms"		// g3
#define USER_DELAY   "3ms"		// g4

#define P_ERROR      0.00001    // probabilidad de error de todos los enlaces

// parámetros de la transmisión de video
#define MIN_START_VIDEO     "0s"
#define MAX_START_VIDEO     "56h" // 3 alarmas por semana -> 3 alarmas/(7*24) horas = 0.017 -> 1 alarm cada 1h/0.017=56h
#define MEAN_DRTN_VIDEO     "5min" //5min de media.
#define INTERVALO_VIDEO     "2ms" //intervalo entre paquetes de video

// parámetros del envío de informes en bits
#define TAM_MEDIO_INFO      2000000

// definiciones generales
#define NUM_USUARIOS        60 //el numero de usuarios representara tambien el numero de camaras 1 usuario -> 1 camara
#define NUM_SERVIDORES      3
#define NUM_PREMIUM         20
#define ENLACES_TRONCALES   2
#define NUM_NODOS_ENLACE    2 //numero de nodos por enlace
#define T_START             "0s"
#define T_STOP              "1h"
// indices para la tablas
#define G1_G3       0
#define G1_G4       1
#define ROUTER      0
#define NODO_FINAL  1

// índices para las tablas de curvas
#define CAM_SERV_PREMIUM  0
#define CAM_SERV_BESTEFF  1
#define CAM_USER_PREMIUM  2
#define CAM_USER_BESTEFF  3

//definimos los identificadores de protocolo de TCP y UDP
#define PROTOCOLO_TCP 6
#define PROTOCOLO_UDP 17
#define PROTOCOLO_IP  21

//definiciones para las graficas.
#define NUM_MUESTRAS 5
#define TSTUDENT 2.7765
#define NUM_PUNTOS 10
#define DELTA_USERS 30
#define DELTA_RATE 2000000 //en bps

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
  Time intervalVideo; // intervalo entre paquetes
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

// Estructura para los resultados de la simulación
typedef struct {
  Average<double> varMaxVideo[4];
  Average<double> retMaxVideo[4];
  Average<double> porPerVideo[4];
  Average<double> retMaxInfo[2];
  Average<double> porPerInfo[2];
} ResultSimulacion;


/************ Definición de funciones *************/
/*
**  Función encargada de la configuración y simulación del escenario
**
**  - Parámetros de entrada:
**
**  - Parámetros de salida:
**
*/

void escenario(ParamsEscenario paramsEscenario, ResultSimulacion * resultados);

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
  Time intervalVideo (INTERVALO_VIDEO);

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
  cmd.AddValue ("intervaloVideo", "Tiempo entre envío de paquetes de la transmisión de video", intervalVideo);
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
  alarmVideo.intervalVideo = intervalVideo;

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

  /** Generación de gráficas **/
  // Gráfica para la variación máxima de retardo en vídeo en función del ratio clientes/servidores
  Gnuplot graficaVarVideoRatio;
  graficaVarVideoRatio.SetLegend ("Ratio clientes/servidores", "Variación retardo máxima (us)");
  graficaVarVideoRatio.SetTitle (std::string("Variación máxima de retardo en vídeo en función del ratio clientes/servidores \\n intervalo entre videos :") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de la camara: ")+std::to_string(camRate.GetBitRate()) + std::string("bps\\n Tamaño de los informes :") + std::to_string(tamMedioInforme) + std::string("bits"));
  // Gráfica para el retardo máximo en vídeo en función del ratio clientes/servidores
  Gnuplot graficaRetVideoRatio;
  graficaRetVideoRatio.SetLegend ("Ratio clientes/servidores", "Retardo máximo (us)");
  graficaRetVideoRatio.SetTitle (std::string("Retardo máximo en vídeo en función del ratio clientes/servidores\\n intervalo entre videos :") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de las cámaras ")+std::to_string(camRate.GetBitRate()) + std::string("bps\\n Tamaño de los informes: ") + std::to_string(tamMedioInforme) + std::string("bits"));
  // Gráfica para el % de paquetes perdidos en vídeo en función del ratio clientes/servidores
  Gnuplot graficaPerVideoRatio;
  graficaPerVideoRatio.SetLegend ("Ratio clientes/servidores", "Paquetes perdidos (%)");
  graficaPerVideoRatio.SetTitle (std::string("Paquetes perdidos en vídeo en función del ratio clientes/servidores\\n intervalo entre videos :") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de las cámaras ")+std::to_string(camRate.GetBitRate()) + std::string("bps\\n Tamaño de los informes :") + std::to_string(tamMedioInforme) + std::string("bits"));
  // Gráfica para el retardo máximo en informes en función del ratio clientes/servidores
  Gnuplot graficaRetInfoRatio;
  graficaRetInfoRatio.SetLegend ("Ratio clientes/servidores", "Retardo máximo (us)");
  graficaRetInfoRatio.SetTitle (std::string("Retardo máximo en informes en función del ratio clientes/servidores \\n intervalo entre videos :") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de las cámaras ")+std::to_string(camRate.GetBitRate()) + std::string("bps\\n Tamaño de los informes :") + std::to_string(tamMedioInforme) + std::string("bits"));
  // Gráfica para el % de paquetes perdidos en informes en función del ratio clientes/servidores
  Gnuplot graficaPerInfoRatio;
  graficaPerInfoRatio.SetLegend ("Ratio clientes/servidores", "Paquetes perdidos (%)");
  graficaPerInfoRatio.SetTitle (std::string("Porcentaje de paquetes perdidos en informes en función del ratio clientes/servidores\\n intervalo entre videos :") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de las cámaras ")+std::to_string(camRate.GetBitRate()) + std::string("bps\\n Tamaño de los informes :") + std::to_string(tamMedioInforme) + std::string("bits"));

  // Gráfica para la variación máxima de retardo en función de la capacidad de los enlaces entre routers
  Gnuplot graficaVarVideoCap;
  graficaVarVideoCap.SetLegend ("Capacidad enlaces routers (Mb/s)", "Variación retardo máxima (us)");
  graficaVarVideoCap.SetTitle (std::string("Variación máxima de retardo en función de la capacidad de los enlaces entre routers\\n intervalo entre videos") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de las cámaras ")+std::to_string(camRate.GetBitRate()) + std::string("bps\\n Tamaño de los informes :") + std::to_string(tamMedioInforme) + std::string("bits"));
  // Gráfica para el retardo máximo en función de la capacidad de los enlaces entre routers
  Gnuplot graficaRetVideoCap;
  graficaRetVideoCap.SetLegend ("Capacidad enlaces routers (Mb/s)", "Retardo máximo (us)");
  graficaRetVideoCap.SetTitle (std::string("Retardo máximo en función de la capacidad de los enlaces entre routers\\n intervalo entre videos") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de las cámaras ")+std::to_string(camRate.GetBitRate()) + std::string("bps\\n Tamaño de los informes :") + std::to_string(tamMedioInforme) + std::string("bits"));
  // Gráfica para el % de paquetes perdidos en función de la capacidad de los enlaces entre routers
  Gnuplot graficaPerVideoCap;
  graficaPerVideoCap.SetLegend ("Capacidad enlaces routers (Mb/s)", "Paquetes perdidos (%)");
  graficaPerVideoCap.SetTitle (std::string("Porcentaje de paquetes perdidos en función de la capacidad de los enlaces entre routers\\n intervalo entre videos") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de las cámaras ")+std::to_string(camRate.GetBitRate()) + std::string("bps\\n Tamaño de los informes :") + std::to_string(tamMedioInforme) + std::string("bits"));
  // Gráfica para el retardo máximo en informes en función de la capacidad de los enlaces entre routers
  Gnuplot graficaRetInfoCap;
  graficaRetInfoCap.SetLegend ("Capacidad enlaces routers (Mb/s)", "Retardo máximo (us)");
  graficaRetInfoCap.SetTitle (std::string("Retardo máximo en informes en función de la capacidad de los enlaces entre routers\\n intervalo entre videos") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de las cámaras ")+std::to_string(camRate.GetBitRate()) + std::string("bps\\n Tamaño de los informes :") + std::to_string(tamMedioInforme) + std::string("bits"));
  // Gráfica para el % de paquetes perdidos en informes en función de la capacidad de los enlaces entre routers
  Gnuplot graficaPerInfoCap;
  graficaPerInfoCap.SetLegend ("Capacidad enlaces routers (Mb/s)", "Paquetes perdidos (%)");
  graficaPerInfoCap.SetTitle (std::string("Porcentaje de paquetes perdidos en informes en función de la capacidad de los enlaces entre routers\\n intervalo entre videos") + std::to_string(intervalVideo.GetSeconds()) + std::string("s.,\\n tiempo de duracion de la simulacion : ")+std::to_string(tstop.GetSeconds()) +std::string("s.\\n, probabilidad de error ") +std::to_string(routerPerror) + std::string("\\n Tasa de envio de las cámaras ")+std::to_string(camRate.GetBitRate()) + std::string("\\n Tamaño de los informes :") + std::to_string(tamMedioInforme) + std::string("bits"));

  /** Simulación **/
  // Necesitaremos 4 curvas para cada gráfica del vídeo
  Gnuplot2dDataset curvasVarVideoRatio[4];
  curvasVarVideoRatio[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasVarVideoRatio[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");
  curvasVarVideoRatio[CAM_USER_PREMIUM] = Gnuplot2dDataset ("Camara -> Usuario Premium");
  curvasVarVideoRatio[CAM_USER_BESTEFF] = Gnuplot2dDataset ("Camara -> Usuario Best-Effort");
  Gnuplot2dDataset curvasRetVideoRatio[4];
  curvasRetVideoRatio[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasRetVideoRatio[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");
  curvasRetVideoRatio[CAM_USER_PREMIUM] = Gnuplot2dDataset ("Camara -> Usuario Premium");
  curvasRetVideoRatio[CAM_USER_BESTEFF] = Gnuplot2dDataset ("Camara -> Usuario Best-Effort");
  Gnuplot2dDataset curvasPerVideoRatio[4];
  curvasPerVideoRatio[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasPerVideoRatio[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");
  curvasPerVideoRatio[CAM_USER_PREMIUM] = Gnuplot2dDataset ("Camara -> Usuario Premium");
  curvasPerVideoRatio[CAM_USER_BESTEFF] = Gnuplot2dDataset ("Camara -> Usuario Best-Effort");
  Gnuplot2dDataset curvasVarVideoCap[4];
  curvasVarVideoCap[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasVarVideoCap[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");
  curvasVarVideoCap[CAM_USER_PREMIUM] = Gnuplot2dDataset ("Camara -> Usuario Premium");
  curvasVarVideoCap[CAM_USER_BESTEFF] = Gnuplot2dDataset ("Camara -> Usuario Best-Effort");
  Gnuplot2dDataset curvasRetVideoCap[4];
  curvasRetVideoCap[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasRetVideoCap[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");
  curvasRetVideoCap[CAM_USER_PREMIUM] = Gnuplot2dDataset ("Camara -> Usuario Premium");
  curvasRetVideoCap[CAM_USER_BESTEFF] = Gnuplot2dDataset ("Camara -> Usuario Best-Effort");
  Gnuplot2dDataset curvasPerVideoCap[4];
  curvasPerVideoCap[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasPerVideoCap[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");
  curvasPerVideoCap[CAM_USER_PREMIUM] = Gnuplot2dDataset ("Camara -> Usuario Premium");
  curvasPerVideoCap[CAM_USER_BESTEFF] = Gnuplot2dDataset ("Camara -> Usuario Best-Effort");

  for (uint8_t curva = 0 ; curva < 4 ; curva++) {
    curvasVarVideoRatio[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasVarVideoRatio[curva].SetErrorBars (Gnuplot2dDataset::Y);
    curvasRetVideoRatio[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasRetVideoRatio[curva].SetErrorBars (Gnuplot2dDataset::Y);
    curvasPerVideoRatio[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasPerVideoRatio[curva].SetErrorBars (Gnuplot2dDataset::Y);
    curvasVarVideoCap[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasVarVideoCap[curva].SetErrorBars (Gnuplot2dDataset::Y);
    curvasRetVideoCap[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasRetVideoCap[curva].SetErrorBars (Gnuplot2dDataset::Y);
    curvasPerVideoCap[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasPerVideoCap[curva].SetErrorBars (Gnuplot2dDataset::Y);
  }

  // y 2 para cada gráfica del informe
  Gnuplot2dDataset curvasRetInfoRatio[2];
  curvasRetInfoRatio[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasRetInfoRatio[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");
  Gnuplot2dDataset curvasPerInfoRatio[2];
  curvasPerInfoRatio[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasPerInfoRatio[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");
  Gnuplot2dDataset curvasRetInfoCap[2];
  curvasRetInfoCap[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasRetInfoCap[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");
  Gnuplot2dDataset curvasPerInfoCap[2];
  curvasPerInfoCap[CAM_SERV_PREMIUM] = Gnuplot2dDataset ("Camara -> Servidor Premium");
  curvasPerInfoCap[CAM_SERV_BESTEFF] = Gnuplot2dDataset ("Camara -> Servidor Best-Effort");

  for (uint8_t curva = 0 ; curva < 2 ; curva++) {
    curvasRetInfoRatio[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasRetInfoRatio[curva].SetErrorBars (Gnuplot2dDataset::Y);
    curvasPerInfoRatio[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasPerInfoRatio[curva].SetErrorBars (Gnuplot2dDataset::Y);
    curvasRetInfoCap[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasRetInfoCap[curva].SetErrorBars (Gnuplot2dDataset::Y);
    curvasPerInfoCap[curva].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    curvasPerInfoCap[curva].SetErrorBars (Gnuplot2dDataset::Y);
  }

  // Realizamos 10 simulaciones variando el ratio clientes/servidores
  uint32_t clientes = paramsEscenario.numUsers;
  for ( uint32_t punto = 0 ; punto < NUM_PUNTOS; punto++) {
    NS_LOG_INFO("Simulando con numero de usuarios: " << clientes);
    // Aumentamos también el número de usuarios premium para mantener la proporción
    // Actualizamos los parámetros de la simulación
    paramsEscenario.numUsers = clientes;
    paramsEscenario.numPremium += numPremium;

    ResultSimulacion resultados;
    // Se realizan varias muestras
    for (uint8_t muestra = 0 ; muestra < NUM_MUESTRAS ; muestra++) {
      NS_LOG_INFO("Muestra número: " << std::to_string(muestra));
      escenario(paramsEscenario, &resultados);
    }

    // Añadimos los resultados a las curvas
    for (uint8_t curva = 0 ; curva < 4 ; curva++) {
      double errorVar = TSTUDENT * sqrt (resultados.varMaxVideo[curva].Var () / resultados.varMaxVideo[curva].Count ());
      double errorRet = TSTUDENT * sqrt (resultados.retMaxVideo[curva].Var () / resultados.retMaxVideo[curva].Count ());
      double errorPer = TSTUDENT * sqrt (resultados.porPerVideo[curva].Var () / resultados.porPerVideo[curva].Count ());
      double ratio = clientes/numServers;
      curvasVarVideoRatio[curva].Add(ratio, resultados.varMaxVideo[curva].Mean(), errorVar);
      curvasRetVideoRatio[curva].Add(ratio, resultados.retMaxVideo[curva].Mean(), errorRet);
      curvasPerVideoRatio[curva].Add(ratio, resultados.porPerVideo[curva].Mean(), errorPer);
    }
    for (uint8_t curva = 0 ; curva < 2 ; curva++) {
      double errorRet = TSTUDENT * sqrt (resultados.retMaxInfo[curva].Var () / resultados.retMaxInfo[curva].Count ());
      double errorPer = TSTUDENT * sqrt (resultados.porPerInfo[curva].Var () / resultados.porPerInfo[curva].Count ());
      double ratio = clientes/numServers;
      curvasRetInfoRatio[curva].Add(ratio, resultados.retMaxInfo[curva].Mean(), errorRet);
      curvasPerInfoRatio[curva].Add(ratio, resultados.porPerInfo[curva].Mean(), errorPer);
    }

    //aumentamos el eje x
    clientes += DELTA_USERS;
  }

  // Realizamos 10 simulaciones variando la capacidad de los enlaces entre routers
  paramsEscenario.numUsers = numUsers;
  paramsEscenario.numPremium = numPremium;
  uint64_t bitRate = paramsEscenario.routerLink.rate.GetBitRate();
  for ( uint32_t punto = 0; punto < NUM_PUNTOS ; punto++) {
    NS_LOG_INFO("Simulando con capacidad de enlace: " << bitRate/(8*1000));
    // Actualizamos los parámetros de la simulación
    DataRate capEnlaces (bitRate);
    paramsEscenario.routerLink.rate = capEnlaces;

    ResultSimulacion resultados;
    // Se realizan varias muestras
    for (uint8_t muestra = 0 ; muestra < NUM_MUESTRAS ; muestra++) {
      NS_LOG_INFO("Muestra número: " << std::to_string(muestra));
      escenario(paramsEscenario, &resultados);
    }

    // Añadimos los resultados a las curvas
    for (uint8_t curva = 0 ; curva < 4 ; curva++) {
      double errorVar = TSTUDENT * sqrt (resultados.varMaxVideo[curva].Var () / resultados.varMaxVideo[curva].Count ());
      double errorRet = TSTUDENT * sqrt (resultados.retMaxVideo[curva].Var () / resultados.retMaxVideo[curva].Count ());
      double errorPer = TSTUDENT * sqrt (resultados.porPerVideo[curva].Var () / resultados.porPerVideo[curva].Count ());
      double capEn = capEnlaces.GetBitRate()/double(8*1000);
      curvasVarVideoCap[curva].Add(capEn, resultados.varMaxVideo[curva].Mean(), errorVar);
      curvasRetVideoCap[curva].Add(capEn, resultados.retMaxVideo[curva].Mean(), errorRet);
      curvasPerVideoCap[curva].Add(capEn, resultados.porPerVideo[curva].Mean(), errorPer);
    }
    for (uint8_t curva = 0 ; curva < 2 ; curva++) {
      double errorRet = TSTUDENT * sqrt (resultados.retMaxInfo[curva].Var () / resultados.retMaxInfo[curva].Count ());
      double errorPer = TSTUDENT * sqrt (resultados.porPerInfo[curva].Var () / resultados.porPerInfo[curva].Count ());
      double capEn = capEnlaces.GetBitRate()/double(8*1000);
      curvasRetInfoCap[curva].Add(capEn, resultados.retMaxInfo[curva].Mean(), errorRet);
      curvasPerInfoCap[curva].Add(capEn, resultados.porPerInfo[curva].Mean(), errorPer);
    }

    //modificamos la tasa.
    bitRate += DELTA_RATE;

  }

  // Añadimos las curvas a las gráficas
  for (uint8_t curva = 0 ; curva < 4 ; curva++) {
    graficaVarVideoRatio.AddDataset (curvasVarVideoRatio[curva]);
    graficaRetVideoRatio.AddDataset (curvasRetVideoRatio[curva]);
    graficaPerVideoRatio.AddDataset (curvasPerVideoRatio[curva]);
    graficaVarVideoCap.AddDataset (curvasVarVideoCap[curva]);
    graficaRetVideoCap.AddDataset (curvasRetVideoCap[curva]);
    graficaPerVideoCap.AddDataset (curvasPerVideoCap[curva]);
  }
  for (uint8_t curva = 0 ; curva < 2 ; curva++) {
    graficaRetInfoRatio.AddDataset (curvasRetInfoRatio[curva]);
    graficaPerInfoRatio.AddDataset (curvasPerInfoRatio[curva]);
    graficaRetInfoCap.AddDataset (curvasRetInfoCap[curva]);
    graficaPerInfoCap.AddDataset (curvasPerInfoCap[curva]);
  }

  // Por último generamos los ficheros
  std::ofstream fich_varVideoRatio ("var-video-ratio.plt");
  graficaVarVideoRatio.GenerateOutput (fich_varVideoRatio);
  fich_varVideoRatio << "pause -1" << std::endl;
  fich_varVideoRatio.close ();
  std::ofstream fich_retVideoRatio ("ret-video-ratio.plt");
  graficaRetVideoRatio.GenerateOutput (fich_retVideoRatio);
  fich_retVideoRatio << "pause -1" << std::endl;
  fich_retVideoRatio.close ();
  std::ofstream fich_perVideoRatio ("per-video-ratio.plt");
  graficaPerVideoRatio.GenerateOutput (fich_perVideoRatio);
  fich_perVideoRatio << "pause -1" << std::endl;
  fich_perVideoRatio.close ();
  std::ofstream fich_varVideoCap ("var-video-cap.plt");
  graficaVarVideoCap.GenerateOutput (fich_varVideoCap);
  fich_varVideoCap << "pause -1" << std::endl;
  fich_varVideoCap.close ();
  std::ofstream fich_retVideoCap ("ret-video-cap.plt");
  graficaRetVideoCap.GenerateOutput (fich_retVideoCap);
  fich_retVideoCap << "pause -1" << std::endl;
  fich_retVideoCap.close ();
  std::ofstream fich_perVideoCap ("per-video-cap.plt");
  graficaPerVideoCap.GenerateOutput (fich_perVideoCap);
  fich_perVideoCap << "pause -1" << std::endl;
  fich_perVideoCap.close ();

  std::ofstream fich_retInfoRatio ("ret-info-ratio.plt");
  graficaRetInfoRatio.GenerateOutput (fich_retInfoRatio);
  fich_retInfoRatio << "pause -1" << std::endl;
  fich_retInfoRatio.close ();
  std::ofstream fich_perInfoRatio ("per-info-ratio.plt");
  graficaPerInfoRatio.GenerateOutput (fich_perInfoRatio);
  fich_perInfoRatio << "pause -1" << std::endl;
  fich_perInfoRatio.close ();
  std::ofstream fich_retInfoCap ("ret-info-cap.plt");
  graficaRetInfoCap.GenerateOutput (fich_retInfoCap);
  fich_retInfoCap << "pause -1" << std::endl;
  fich_retInfoCap.close ();
  std::ofstream fich_perInfoCap ("per-info-cap.plt");
  graficaPerInfoCap.GenerateOutput (fich_perInfoCap);
  fich_perInfoCap << "pause -1" << std::endl;
  fich_perInfoCap.close ();

  return 0;
}


void escenario(ParamsEscenario paramsEscenario, ResultSimulacion * resultados){
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
  Ipv4InterfaceContainer interfacesGrupo1 [paramsEscenario.numServers];
  uint32_t serverMaxIp;
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numServers; n_nodo++){
        Ipv4AddressHelper addressesGrupo1;
        std::string dir = ("10.1."+std::to_string(n_nodo+1)+".0");
        NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 1: "<<dir);
        char *dir_buff = new char[dir.length()]; // dir.length() es el tamano que tendra la ip
        strcpy(dir_buff, dir.c_str());
        Ipv4Address ipv4Address ("0.0.0.0");
        ipv4Address.Set(dir_buff);
        addressesGrupo1.SetBase(ipv4Address, "255.255.255.0");
        interfacesGrupo1[n_nodo] = addressesGrupo1.Assign (devicesGrupo1[n_nodo]);
        if (n_nodo == paramsEscenario.numServers-1)
          serverMaxIp = ipv4Address.Get();
  }
  // Asignamos direcciones al grupo2
  for(uint32_t n_nodo = 0; n_nodo < ENLACES_TRONCALES; n_nodo++){
        Ipv4AddressHelper addressesGrupo2;
        Ipv4InterfaceContainer interfacesGrupo2;
        std::string dir = ("10.2."+std::to_string(n_nodo+1)+".0");
        NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 2: "<<dir);
        char *dir_buff = new char[dir.length()];// dir.length() es el tamano que tendra la ip
        strcpy(dir_buff, dir.c_str());
        Ipv4Address ipv4Address ("0.0.0.0");
        ipv4Address.Set(dir_buff);
        addressesGrupo2.SetBase(ipv4Address, "255.255.255.0");
        interfacesGrupo2 = addressesGrupo2.Assign (devicesGrupo2[n_nodo]);
  }

  // Asignamos direcciones al grupo3 y grupo4
  Ipv4Address ipCamaras[paramsEscenario.numUsers];//para almacenar las direcciones ip de las camaras.
  Ipv4InterfaceContainer interfacesGrupo3[paramsEscenario.numUsers];
  Ipv4InterfaceContainer interfacesGrupo4[paramsEscenario.numUsers];
  uint32_t n_nodo = 0;
  uint32_t premiumMaxIp;//para almacenar la ip del ultimo nodo premium
  for(uint32_t segundaPosIP = 0; segundaPosIP <= 255 && n_nodo < paramsEscenario.numUsers ; segundaPosIP++){
    for(uint32_t terceraPosIP = 0; terceraPosIP <= 255 && n_nodo < paramsEscenario.numUsers ; terceraPosIP++){

      Ipv4AddressHelper addressesGrupo3;
      Ipv4AddressHelper addressesGrupo4;

      std::string dir = "13."+std::to_string(segundaPosIP)+"."+std::to_string(terceraPosIP)+".0";
      NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 3: "<<dir);
      char *dir_buff = new char[dir.length()];// dir.length() es el tamano que tendra la ip
      strcpy(dir_buff, dir.c_str());
      Ipv4Address ipv4Address ("0.0.0.0");
      ipv4Address.Set(dir_buff);
      addressesGrupo3.SetBase(ipv4Address, "255.255.255.0");
      interfacesGrupo3[n_nodo] = addressesGrupo3.Assign (devicesGrupo3[n_nodo]);
      //guardamos la ip de la camara, la ip de la camara sera la terminada en .2.
      dir = "13."+std::to_string(segundaPosIP)+"."+std::to_string(terceraPosIP)+".2";
      dir_buff = new char[dir.length()];
      ipCamaras[n_nodo] = Ipv4Address();
      strcpy(dir_buff, dir.c_str());
      ipCamaras[n_nodo].Set(dir_buff);

      dir = "14."+std::to_string(segundaPosIP)+"."+std::to_string(terceraPosIP)+".0";
      NS_LOG_DEBUG("Direccion IP enlace "<<n_nodo<<" Grupo 4: "<<dir);
      dir_buff = new char[dir.length()];// dir.length() es el tamano que tendra la ip
      strcpy(dir_buff, dir.c_str());
      ipv4Address.Set(dir_buff);
      addressesGrupo4.SetBase(ipv4Address, "255.255.255.0");
      interfacesGrupo4[n_nodo] = addressesGrupo4.Assign (devicesGrupo4[n_nodo]);

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
  // Añadimos un filtro de paquetes para encolar por prioridad
  trafficControl.AddPacketFilter (handle, "ns3::PrioPacketFilter", "PremiumMaxIp", UintegerValue(premiumMaxIp));

  // Instalamos el controlador de tráfico en cada router
  NetDeviceContainer routerDevices;
  routerDevices.Add(devicesGrupo2[G1_G3].Get(0));
  routerDevices.Add(devicesGrupo2[G1_G3].Get(1));
  routerDevices.Add(devicesGrupo2[G1_G4].Get(1));
  trafficControl.Uninstall(routerDevices);
  trafficControl.Install(routerDevices);

  ////FIN CONFIGURACION COLAS////

  // Poblamos las tablas de rutas
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /*__________________________________________**
  ** Creacion de aplicaciones y configuracion **
  **------------------------------------------**/
  // creamos helpers para crear los sumideros de video y de informes.
  uint16_t videoPort = 9;
  uint16_t informePort = 10;
  PacketSinkHelper receptorVideoHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), videoPort)));
  PacketSinkHelper receptorInformeHelper ("ns3::TcpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), informePort)));

  // instalamos los sumideros en los servidores
  for (uint32_t n_server = 0; n_server < paramsEscenario.numServers; n_server++) {
    ApplicationContainer receptorVideoServidor;
    ApplicationContainer receptorInformeServidor;
    receptorVideoServidor = receptorVideoHelper.Install(grupoUno[n_server].Get(NODO_FINAL));
    receptorVideoServidor.Start(paramsEscenario.tstart);
    receptorInformeServidor = receptorInformeHelper.Install(grupoUno[n_server].Get(NODO_FINAL));
    receptorInformeServidor.Start(paramsEscenario.tstart);
  }

  // instalamos los sumideros en los usuarios.
  for (uint32_t n_user = 0; n_user < paramsEscenario.numUsers; n_user++) {
    ApplicationContainer receptorVideoUsuario;
    ApplicationContainer receptorInformeUsuario;
    receptorVideoUsuario = receptorVideoHelper.Install(grupoCuatro[n_user].Get(NODO_FINAL));
    receptorVideoUsuario.Start(paramsEscenario.tstart);
    receptorInformeUsuario = receptorInformeHelper.Install(grupoCuatro[n_user].Get(NODO_FINAL));
    receptorInformeUsuario.Start(paramsEscenario.tstart);
  }

  //Array qe contiene el numero de servidor al que esta conectado el usuario indice de la tabla.
  uint32_t connServUser[paramsEscenario.numUsers];

  // configuramos e instalamos las aplicaciones en las camaras y los servidores, las camaras y usuarios respectivos estaran
  // conectadas al mismo servidor, ademas, para simular el streaming se iniciaran a la VEZ <----- Esto se va a cambiar ???
  // la transmision desde la camara al servidor y desde el servidor al usuario.
  for (uint32_t n_nodo = 0; n_nodo < paramsEscenario.numUsers; n_nodo++) {
    // Aplicaciones de transmisión
    ApplicationContainer appInformeCam2Server; // envío periódico de informe de la cámara al servidor
    ApplicationContainer appAlarmVideoCam2Server; // streaming de vídeo por alarma al servidor
    ApplicationContainer appAlarmVideoCam2User; // streaming de vídeo por alarma al usuario
    // v.a. para modelar el momento y duracion de la alarma (robo o falsa)
    Ptr<UniformRandomVariable> alarmStart = CreateObject<UniformRandomVariable>();
    Ptr<ExponentialRandomVariable> alarmTime = CreateObject<ExponentialRandomVariable>();
    alarmTime->SetAttribute("Mean", DoubleValue(paramsEscenario.alarmVideo.meanDrtnVideo.GetDouble()));
    //v.a. para modelar el tamano y momento de envio del informes
    Ptr<UniformRandomVariable> momentoEnvioInforme = CreateObject<UniformRandomVariable>();
    Ptr<ExponentialRandomVariable> informeSize = CreateObject<ExponentialRandomVariable>();
    informeSize->SetAttribute("Mean", DoubleValue(paramsEscenario.infoReport.tamMedioInforme));

    // obtenemos y guardamos los valores de incio de las alarmas y su duracion.
    Time inicioAlarma (alarmStart->GetValue(paramsEscenario.alarmVideo.minStartVideo.GetDouble(), paramsEscenario.alarmVideo.maxStartVideo.GetDouble()));
    NS_LOG_DEBUG("Inicio alarma: " << inicioAlarma);
    Time duracionAlarma (alarmTime->GetValue());
    NS_LOG_DEBUG("Duracion alarma: " << duracionAlarma);
    // guardamos los valores de incio de transmision y de tamano de los informe
    uint64_t tamInformes = (uint64_t) informeSize->GetValue();
    NS_LOG_DEBUG("Tamaño informe: " << tamInformes);
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
    BulkSendHelper emisorInformeCam2Server = BulkSendHelper("ns3::TcpSocketFactory", Address (InetSocketAddress (interfacesGrupo1[conServer].GetAddress(NODO_FINAL), informePort)));

    // configuramos con las v.a. el emisor de video en streaming cuando hay alarma.
    emisorAlarmVideoCam2User.SetAttribute("StartTime", TimeValue(inicioAlarma));
    emisorAlarmVideoCam2User.SetAttribute("StopTime", TimeValue(inicioAlarma + duracionAlarma));
    emisorAlarmVideoCam2User.SetAttribute("Interval", TimeValue(paramsEscenario.alarmVideo.intervalVideo));
    emisorAlarmVideoCam2User.SetAttribute("MaxPackets", UintegerValue(4294967295)); // ponemos el maximo numero de paquetes de un entero para que termine cuando acabe el tiempo.
    emisorAlarmVideoCam2Server.SetAttribute("StartTime", TimeValue(inicioAlarma));
    emisorAlarmVideoCam2Server.SetAttribute("StopTime", TimeValue(inicioAlarma + duracionAlarma));
    emisorAlarmVideoCam2Server.SetAttribute("Interval", TimeValue(paramsEscenario.alarmVideo.intervalVideo));
    emisorAlarmVideoCam2Server.SetAttribute("MaxPackets", UintegerValue(4294967295));

    // configuramos los helpers para emision de informes.
    emisorInformeCam2Server.SetAttribute("StartTime", TimeValue(inicioTxInforme));
    emisorInformeCam2Server.SetAttribute("SendSize", UintegerValue(1024));
    emisorInformeCam2Server.SetAttribute("MaxBytes", UintegerValue(tamInformes));


    // realizamos las instalaciones.
    appAlarmVideoCam2Server = emisorAlarmVideoCam2Server.Install(grupoTres[n_nodo]);
    appAlarmVideoCam2User = emisorAlarmVideoCam2User.Install(grupoTres[n_nodo]);
    appInformeCam2Server = emisorInformeCam2Server.Install(grupoTres[n_nodo]);

  }

  /** Creación de observadores **/
  //observadores para los usuarios premium.
  Observador * obsPremium[paramsEscenario.numPremium];
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numPremium; n_nodo++){
    ParametrosObservador params;
    params.camara = devicesGrupo3[n_nodo].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.usuario = devicesGrupo4[n_nodo].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.servidor = devicesGrupo1[connServUser[n_nodo]].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.maxIpServidor = serverMaxIp;
    params.ip = ipCamaras[n_nodo];
    obsPremium[n_nodo] = new Observador(params);
  }
  //observadores para los usuarios no premium.
  Observador * obsNoPremium[paramsEscenario.numUsers - paramsEscenario.numPremium];
  for(uint32_t n_nodo = paramsEscenario.numPremium; n_nodo < paramsEscenario.numUsers; n_nodo++){
    ParametrosObservador params;
    params.camara = devicesGrupo3[n_nodo].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.usuario = devicesGrupo4[n_nodo].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.servidor = devicesGrupo1[connServUser[n_nodo]].Get(NODO_FINAL)->GetObject<PointToPointNetDevice>();
    params.maxIpServidor = serverMaxIp;
    params.ip = ipCamaras[n_nodo];
    obsNoPremium[n_nodo - paramsEscenario.numPremium] = new Observador(params);
  }

  //activamos el pcap.
  //enlaceRouters.EnablePcapAll("routers", true);


  //paramos la simulacion en el tstop ya que los eventos simulados pueden haberse configurado tras las 3h analizadas,
  //puesto que hemos adaptado la probabilidad de que salte una alarma.
  Simulator::Stop(paramsEscenario.tstop);
  Simulator::Run();

  Average<double> varMaxRetVidCam2Usr;
  Average<double> retMedVidCam2Usr;
  Average<double> perdidasVidCam2Usr;
  Average<double> varMaxRetVidCam2Serv;
  Average<double> retMedVidCam2Serv;
  Average<double> perdidasVidCam2Serv;
  Average<double> retMedInfCam2Serv;
  Average<double> perdidasInfCam2Serv;
  // Obtenemos los estadísticos de los usuarios premium
  for(uint32_t n_nodo = 0; n_nodo < paramsEscenario.numPremium; n_nodo++){
    obsPremium[n_nodo]->GetEstadisticos(&varMaxRetVidCam2Usr, &retMedVidCam2Usr, &perdidasVidCam2Usr, &varMaxRetVidCam2Serv, &retMedVidCam2Serv, &perdidasVidCam2Serv, &retMedInfCam2Serv, &perdidasInfCam2Serv);
  }
  resultados->varMaxVideo[CAM_SERV_PREMIUM].Update(varMaxRetVidCam2Serv.Max());
  resultados->varMaxVideo[CAM_USER_PREMIUM].Update(varMaxRetVidCam2Usr.Max());
  resultados->retMaxVideo[CAM_SERV_PREMIUM].Update(retMedVidCam2Serv.Max());
  resultados->retMaxVideo[CAM_USER_PREMIUM].Update(retMedVidCam2Usr.Max());
  resultados->retMaxInfo[CAM_SERV_PREMIUM].Update(retMedInfCam2Serv.Max());
  resultados->porPerVideo[CAM_SERV_PREMIUM].Update(perdidasVidCam2Serv.Max());
  resultados->porPerVideo[CAM_USER_PREMIUM].Update(perdidasVidCam2Usr.Max());
  resultados->porPerInfo[CAM_SERV_PREMIUM].Update(perdidasInfCam2Serv.Max());
  // Reseteamos los average
  varMaxRetVidCam2Usr.Reset();
  retMedVidCam2Usr.Reset();
  perdidasVidCam2Usr.Reset();
  varMaxRetVidCam2Serv.Reset();
  retMedVidCam2Serv.Reset();
  perdidasVidCam2Serv.Reset();
  retMedInfCam2Serv.Reset();
  perdidasInfCam2Serv.Reset();
  // Obtenemos los estadísticos de los usuarios no premium
  for(uint32_t n_nodo = 0; n_nodo < (paramsEscenario.numUsers - paramsEscenario.numPremium); n_nodo++){
    obsNoPremium[n_nodo]->GetEstadisticos(&varMaxRetVidCam2Usr, &retMedVidCam2Usr, &perdidasVidCam2Usr, &varMaxRetVidCam2Serv, &retMedVidCam2Serv, &perdidasVidCam2Serv, &retMedInfCam2Serv, &perdidasInfCam2Serv);
  }
  resultados->varMaxVideo[CAM_SERV_BESTEFF].Update(varMaxRetVidCam2Serv.Max());
  resultados->varMaxVideo[CAM_USER_BESTEFF].Update(varMaxRetVidCam2Usr.Max());
  resultados->retMaxVideo[CAM_SERV_BESTEFF].Update(retMedVidCam2Serv.Max());
  resultados->retMaxVideo[CAM_USER_BESTEFF].Update(retMedVidCam2Usr.Max());
  resultados->retMaxInfo[CAM_SERV_BESTEFF].Update(retMedInfCam2Serv.Max());
  resultados->porPerVideo[CAM_SERV_BESTEFF].Update(perdidasVidCam2Serv.Max());
  resultados->porPerVideo[CAM_USER_BESTEFF].Update(perdidasVidCam2Usr.Max());
  resultados->porPerInfo[CAM_SERV_BESTEFF].Update(perdidasInfCam2Serv.Max());

  //destruimos la simulacion.
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
  // Cola en la que se encolará el paquete
  int32_t cola = 0;

  Ptr<Ipv4QueueDiscItem> packet = (dynamic_cast<Ipv4QueueDiscItem *> (PeekPointer (item)));

  Ipv4Header ipHeader = packet->GetHeader();
  //ip origen del paquete
  Ipv4Address addr = ipHeader.GetSource();
  NS_LOG_DEBUG(addr);
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
