#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "..//shared/protocolo.h"

#define ALGORITMO_SJF_CD 1;
#define ALGORITMO_SJF_SD 2;
#define ALGORITMO_HRRN 3;

typedef struct {
	int puerto;
	int algoritmoPlanificacion;
	int estimacionInicial;
	char* ipCoordinador;
	int puertoCoordinador;
} t_configuracion;

typedef struct{
	char* mensaje;

}tSolicitudESI;

t_configuracion* configuracion;
t_config* fd_configuracion;
t_log *logger;
pthread_t hiloConsola;

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int codigo);
void levantarConsola();
void consola();
void escucharESIs();

//metodos de la consola
void pause();
void play();
void lock(char *comando);
void unlock(char *comando);
void list(char *comando);
void kill(char *comando);
void status(char *comando);
void deadlock();



#endif /* PLANIFICADOR_H_ */
