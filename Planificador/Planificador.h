#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

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

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int codigo);
void consola();



#endif /* PLANIFICADOR_H_ */
