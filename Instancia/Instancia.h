#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h> // strerror
#include <time.h> // time
#include <unistd.h>  // alarm
#include <ctype.h> // toupper
#include <dirent.h>
#include <sys/mman.h> // mmap
#include <sys/types.h>
#include <sys/stat.h> // mkdir - stat
#include <sys/timerfd.h> // create_timer
#include <fcntl.h>
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
	char czNomProc[20]; // nombre para mostrar en el sistema
	char *nombreDeInstancia;

	unsigned int cantidadEntradas;
	unsigned int tamanioEntrada; // bytes
	char *puntoDeMontaje;
	int algoritmoDeReemplazo;

	struct itimerspec intervaloDump; // segundos
	int fdTimerDump;
	time_t ultimoDump;

	char *ipCoordinador;
	unsigned int puertoCoordinador;
	int fdSocketCoordinador;

	unsigned int instruccionesProcesadas;
} t_confInstancia;

/* estructura que representa una entrada en la tabla */
typedef struct {
	char clave[MAX_LONG_CLAVE];
	size_t tamanio;
	unsigned int ultimaInstruccion; // coincide con instruccionesProcesadas al momento de crearse
} t_entrada;

// TODO revisar si es necesario que todas estas sean globales (mala práctica, muy mala)
t_commandLineParameters parametrosEntrada;
t_confInstancia configuracion;
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
unsigned int entradasDisponibles(void);
void limpiarTablaDeEntradas(void);
int inicializarPuntoDeMontaje(void);
void cargarEntradasDesdeArchivos(char **);
char * valorDeEntrada(unsigned int);
void deprecated_cargarEntradasDesdeArchivos(void);
void ejecutarSet(char *, char *, int);

// pantalla [PantallaInstancia.c]
void pantallaInicio(void);
void mostrarConfiguracion(void);
void mostrarConexionCoordinador(void);
void mostrarEstadoTablaDeEntradas(void);
void listarEntradas(void);
void pantallaFin(void);
void mostrarTexto(char *);
void mostrarMenu(void);

#endif /* INSTANCIA_H_ */
