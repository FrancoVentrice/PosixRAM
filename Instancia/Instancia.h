#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/process.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h> // mkdir
#include <unistd.h>  // alarm

#define ALGORITMO_CIRC 1
#define ALGORITMO_LRU 2
#define ALGORITMO_BSU 3

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

t_confInstancia * configuracion;
t_config * fd_configuracion;
t_log * logger;

void iniciarLogger();
int cargarConfiguracion(char *);
void finalizar(int);
int configValida(t_config *);
void iniciarDumpTimeout(unsigned int);
void capturaSenial(int);

#endif /* INSTANCIA_H_ */
