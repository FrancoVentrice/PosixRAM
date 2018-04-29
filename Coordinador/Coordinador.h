#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

#define ALGORITMO_LSU 1;
#define ALGORITMO_EL 2;
#define ALGORITMO_KE 3;

typedef struct {
	int puertoEscucha;
	int algoritmoDistribucion;
	int cantidadDeEntradas;
	int tamanioDeEntrada;
	int retardo;
} t_configuracion;

typedef struct{
	char * mensaje;
} tSolicitudESI;

typedef struct{
	char * mensaje;
} tRespuesta;


t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int);
int configValida(t_config *);

#endif /* COORDINADOR_H_ */
