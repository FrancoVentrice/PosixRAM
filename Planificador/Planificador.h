#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/list.h>
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
	int alfa;
} t_configuracion;

typedef struct{
	char* mensaje;

}tSolicitudESI;



typedef struct{
	char* mensaje;

}tSolicitudPlanificador;

typedef struct{
	char* mensaje;

}tRespuesta;

typedef struct {
	int id;
	float estimacion; //se puede usar con los dos algoritmos
	int rafagaAnterior; //necesario para SJF. Se suma uno cada vez que el ESI ejecuta correctamente una sentencia
	float estimacionAnterior;//necesario para SJF
	int socket; //socket que se esta usando para la comunicacion con el ESI en particular
} t_esi;

t_configuracion* configuracion;
t_config* fd_configuracion;
t_log *logger;
pthread_t hiloConsola;

t_esi *esiEnEjecucion; //vendria a ser la "cola" de ejecucion
t_list* colaDeListos;

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int codigo);
void levantarConsola();
void consola();
void escucharESIs();

//metodos de SJF
void estimarSJF();
void sentenciaEjecutadaCorrectamenteSJF();

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
