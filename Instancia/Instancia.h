#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h> // time
#include <sys/stat.h> // mkdir
#include <unistd.h>  // alarm
#include <ctype.h> // toupper
#include <sys/mman.h> // mmap
#include <commons/config.h>
#include <commons/log.h>
#include <commons/process.h>
#include <commons/string.h>
#include "../shared/libgral.h"
#include "../shared/pantalla.h"
#include "../shared/sockets.h"
#include "../shared/serializar.h"

#define ALGORITMO_CIRC 1
#define ALGORITMO_LRU 2
#define ALGORITMO_BSU 3

#define MAX_LONG_CLAVE 40+1

/* estructura para los valores de la línea de comandos */
typedef struct {
	char *archivoConf;
	int debugMode; // 1 = ON . 0 = OFF (default)
	int logPantalla; // 1 = ON . 0 = OFF (default)
} t_commandLineParameters;

/* estructura para la configuración general */
typedef struct {
	char *nombreDeInstancia;
	char *puntoDeMontaje;
	int algoritmoDeReemplazo;
	unsigned int intervaloDump; // segundos
	char *ipCoordinador;
	unsigned int puertoCoordinador;
	int fdSocketCoordinador;
	unsigned int cantidadEntradas;
	unsigned int tamanioEntrada; // bytes
} t_confInstancia;

/* estructura para llevar el estado y las estadísticas */
typedef struct {
	char czNomProc[20]; // nombre para mostrar en el sistema
	int realizarDump; // 1 = realizar dump . 0 = no realizar dump
	time_t ultimoDump;
	unsigned int instruccionesProcesadas;
	//unsigned int cantidadEntradasOcupadas;
	//unsigned int espacioOcupado; // bytes
} t_instanceStatus;

/* estructura que representa una entrada en la tabla */
typedef struct {
	char clave[MAX_LONG_CLAVE];
	unsigned int tamanio;
	unsigned int ultimaInstruccion; // coincide con instruccionesProcesadas al momento de crearse
} t_entrada;

// TODO revisar si es necesario que todas estas sean globales (mala práctica, muy mala)
t_commandLineParameters parametrosEntrada;
t_confInstancia * configuracion;
t_instanceStatus estadoInstancia;
t_config * fd_configuracion;
t_log * logger;

t_entrada * tablaDeEntradas;
char * almacenamientoEntradas;

// inicio [CoreInstancia.c]
int procesarLineaDeComandos (int, char **);
void inicializarInstancia(void);
void iniciarLogger(void);
void finalizar(int);

// configuracion [ConfigInstancia.c]
int cargarConfiguracion(void);
int configValida(t_config *);
void limpiarConfiguraion(void);

// procesamiento
int conectarACoordinador(void);
void capturaSenial(int);

// manejo de entrads [EntradasInstancia.c]
void prepararTablaDeEntradas(void);
void iniciarDumpTimeout(void);
void volcarEntradas(void);
unsigned int espacioDisponible(void);
void limpiarTablaDeEntradas(void);

// pantalla [PantallaInstancia.c]
void pantallaInicio(void);
void mostrarConfiguracion(void);
void mostrarConexionCoordinador(void);
void mostrarTablaDeEntradas(void);
void pantallaFin(void);
void mostrarTexto(char *);
void mostrarMenu(void);

#endif /* INSTANCIA_H_ */
