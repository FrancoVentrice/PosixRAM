#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h> // mkdir
#include <unistd.h>  // alarm
#include <ctype.h> // toupper
#include <commons/config.h>
#include <commons/log.h>
#include <commons/process.h>
#include <commons/string.h>
#include "..//shared/libgral.h"
#include "..//shared/sockets.h"
#include "..//shared/serializar.h"

#define ALGORITMO_CIRC 1
#define ALGORITMO_LRU 2
#define ALGORITMO_BSU 3

#define STDIN 0

typedef struct {
	char *archivoConf;
	int debugMode; // 1 = ON . 0 = OFF (default)
	int logPantalla; // 1 = ON . 0 = OFF (default)
} t_commandLineParameters;

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

char czNomProc[20]; // nombre para mostrar en el sistema
int realizarDump;

t_commandLineParameters * parametrosEntrada;
t_confInstancia * configuracion;
t_config * fd_configuracion;
t_log * logger;

void iniciarLogger(void);
int cargarConfiguracion(void);
void finalizar(int);
int configValida(t_config *);
void iniciarDumpTimeout(void);
void capturaSenial(int);
void volcarEntradas(void);
int procesarLineaDeComandos (int, char **);
int conectarACoordinador(void);

#endif /* INSTANCIA_H_ */
