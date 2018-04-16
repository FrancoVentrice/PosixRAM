#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

#define ALGORITMO_CIRC 1;
#define ALGORITMO_LRU 2;
#define ALGORITMO_BSU 3;

typedef struct {
	char *ipCoordinador;
	int puertoCoordinador;
	int algoritmoDeReemplazo;
	char *puntoDeMontaje;
	char *nombreDeInstancia;
	int intervaloDump;
} t_configuracion;

t_configuracion* configuracion;
t_config* fd_configuracion;
t_log *logger;

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int codigo);

#endif /* INSTANCIA_H_ */
