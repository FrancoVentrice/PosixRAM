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

t_configuracion* configuracion;
t_config* fd_configuracion;
t_log *logger;

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int codigo);



#endif /* ESI_H_ */
