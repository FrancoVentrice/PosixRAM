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
	int puerto;
	int algoritmoDistribucion;
	int cantidadDeEntradas;
	int tamanioDeEntrada;
	int retardo;
} t_configuracion;

t_configuracion* configuracion;
t_config* fd_configuracion;
t_log *logger;

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int codigo);

#endif /* COORDINADOR_H_ */
