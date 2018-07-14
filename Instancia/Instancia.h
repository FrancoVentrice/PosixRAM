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
#include <stdbool.h>
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
#include "../shared/protocolo.h"
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
	unsigned int instruccionesProcesadas;
	int indiceCIRC;

	struct itimerspec intervaloDump; // segundos
	int fdTimerDump;
	time_t ultimoDump;

	char *ipCoordinador;
	int puertoCoordinador;
	int fdSocketCoordinador;
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

// demasiadas globales (mala práctica, muy mala)
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
int atenderSetClaveValor(char *);

// manejo de entrads [EntradasInstancia.c]
int inicializarPuntoDeMontaje(void);
void prepararTablaDeEntradas(void);
int procesarClavesYCargarEntradas(char *);
void iniciarDumpTimeout(void);
void volcarEntradasEnArchivos(void);
void storeClave(int);
void limpiarTablaDeEntradas(void);
void cargarEntradasDesdeArchivos(char *);
int entradasDisponibles(void);
char * valorDeEntradaPorIndice(int);
char * valorDeEntradaPorClave(char *);
bool existeClave(char *);
bool esEntradaAtomica(int);
int indiceClave(char *);
int realizarCompactacion(void);
int buscarEspacioContiguoDeEntradas(int);
int setClaveValor(char *, char *, t_respuestaSet *);
int entradasOcupadasPorClave(int indice);
int entradasOcupadasPorValor(size_t);

// algoritmos de reemplazo [AlgoReemplazoInstancia.c]
bool existenEntradasAtomicasParaReemplazar(int);
void ejecutarReemplazo(int, t_respuestaSet *);
void reemplazoBSU(int, t_respuestaSet *);
void reemplazoLRU(int, t_respuestaSet *);
void reemplazoCircular(int, t_respuestaSet *);

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

#endif /* INSTANCIA_H_ */
