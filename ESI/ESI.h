#ifndef ESI_H_
#define ESI_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char *ipCoordinador;
	int puertoCoordinador;
	char *ipPlanificador;
	int puertoPlanificador;
} t_configuracion;

typedef struct{
	char * mensaje;
} tSolicitudESI;

typedef struct{
	char* mensaje;
} tRespuestaPlanificador;

t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int);
int configValida(t_config *);

#endif /* ESI_H_ */
