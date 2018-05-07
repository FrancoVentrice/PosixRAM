#ifndef ESI_H_
#define ESI_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char *ipCoordinador;
	int puertoCoordinador;
	int socketCoordinador;
	char *ipPlanificador;
	int puertoPlanificador;
	int socketPlanificador;
} t_configuracion;

/* ******************************************** */
/* ******************************************** */
/* ¿por qué estas dos estructuras son iguales, y ni siquiera son estructuras porque
 * solo tienen una variable adentro?
 * TODO corregir esto
 */

typedef struct{
	char * mensaje;
} tSolicitudESI;

typedef struct{
	char* mensaje;
} tRespuestaPlanificador;

/* ******************************************** */
/* ******************************************** */


t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int);
int configValida(t_config *);
void iniciarConexiones();

#endif /* ESI_H_ */
