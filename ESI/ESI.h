#ifndef ESI_H_
#define ESI_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <parsi/parser.h>

#define OPERACION_GET 1
#define OPERACION_SET 2
#define OPERACION_STORE 3

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

typedef struct{
	char* mensaje;
} tRespuestaCoordinador;

typedef struct{
	int operacion;
	char* clave;
	char* valor;
} t_operacionESI;
/* ******************************************** */
/* ******************************************** */


t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;
FILE * archivo;

size_t *n; //necesaria para ir leyendo el archivo
char *lineptr; //necesaria para ir leyendo el archivo
t_operacionESI *operacion;
bool lecturaRechazada; //cuando una operacion es rechazada (ej: un get de un recurso tomado), sirve para volver a mandar la misma instruccion
char* idESI;

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int);
int configValida(t_config *);
void iniciarConexiones();
void cargarArchivo(char *);
void ordenRecibida();
int leerLinea();
void enviarOperacion();
void enviarEsiFinalizado();

#endif /* ESI_H_ */
