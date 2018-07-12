#ifndef ESI_H_
#define ESI_H_

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <commons/config.h>
#include <commons/log.h>
#include <parsi/parser.h>
#include "../shared/protocolo.h"
#include "../shared/sockets.h"
#include "../shared/serializar.h"
#include "../shared/libgral.h"
#include "../shared/logExtra.h"

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

typedef struct{
	char* mensaje;
} tRespuestaCoordinador;

typedef struct{
	int operacion;
	char* clave;
	char* valor;
} t_operacionESI;

t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;
FILE * archivoScript;

size_t lineLong; //necesaria para ir leyendo el archivo
char *linePtr; //necesaria para ir leyendo el archivo
t_operacionESI *operacion;
bool lecturaRechazada; //cuando una operacion es rechazada (ej: un get de un recurso tomado), sirve para volver a mandar la misma instruccion
bool ejecucion;
char* idESI;

int cargarConfiguracion();
void limpiarConfiguracion();
void cicloPrincipal();
void finalizar(int);
int configValida(t_config *);
void iniciarConexiones();
void cargarArchivo(char *);
void ordenRecibida();
int leerLinea();
void enviarOperacion();
void enviarEsiFinalizado();
void esperarOrdenDeEjecucion();
void enviarLineaOK();
tRespuestaCoordinador * recibirResultadoOperacion();

#endif /* ESI_H_ */
