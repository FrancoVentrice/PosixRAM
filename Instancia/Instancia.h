#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h> // strerror
#include <time.h> // time
#include <unistd.h>  // alarm lseek
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

	int cantidadEntradas;
	int tamanioEntrada; // bytes
	char *puntoDeMontaje;
	int algoritmoDeReemplazo;

	struct itimerspec intervaloDump; // segundos
	int fdTimerDump;
	time_t ultimoDump;

	char *ipCoordinador;
	int puertoCoordinador;
	int fdSocketCoordinador;

	unsigned int instruccionesProcesadas;
} t_confInstancia;

/* estructura que representa una entrada en la tabla */
typedef struct {
	char clave[MAX_LONG_CLAVE];
	size_t tamanio;
	unsigned int ultimaInstruccion; // coincide con instruccionesProcesadas
} t_entrada;

/* estructura para ir almacenando el resultado del SET */
typedef struct {
	char claveReemplazada[MAX_LONG_CLAVE];
	char compactacionRequerida;
} t_respuestaSet;

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

// conexiones [ComunicacionesInstancia.c]
int conectarACoordinador(char **);
int atenderEstadoClave(char *);
int atenderStoreClave(char *);
int atenderEjecutarCompactacion(void);

// manejo de entrads [EntradasInstancia.c]
void prepararTablaDeEntradas(void);
void iniciarDumpTimeout(void);
void volcarEntradasEnArchivos(void);
void storeClave(int);
int entradasDisponibles(void);
void limpiarTablaDeEntradas(void);
int inicializarPuntoDeMontaje(void);
void cargarEntradasDesdeArchivos(char *);
char * valorDeEntradaPorIndice(int);
char * valorDeEntradaPorClave(char *);
int procesarClavesYCargarEntradas(char *);
int existeClave(char *);
int indiceClave(char *);

// pantalla [PantallaInstancia.c]
void pantallaInicio(void);
void mostrarConfiguracion(void);
void mostrarConexionCoordinador(void);
void mostrarEstadoTablaDeEntradas(void);
void listarEntradas(void);
void pantallaFin(void);
void mostrarTexto(char *);
void mostrarMenu(void);
void refrescarPantalla(void);


// deprecated
void deprecated_cargarEntradasDesdeArchivos(void);
char * deprecated_sincronizarClavesConCoordinador(void);
int deprecated_sincronizarClavesYCargarEntradas(void);
void deprecated_ejecutarSet(char *, char *, int);
void deprecated_enviarMensajeOK(void);

#endif /* INSTANCIA_H_ */
