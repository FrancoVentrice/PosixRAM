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

t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;
FILE * archivo;

size_t n; //necesaria para ir leyendo el archivo
char *lineptr; //necesaria para ir leyendo el archivo
t_operacionESI *operacion;
bool lecturaRechazada; //cuando una operacion es rechazada (ej: un get de un recurso tomado), sirve para volver a mandar la misma instruccion
bool ejecucion;
char* idESI;

/* estructura para los valores de la línea de comandos */
typedef struct {
	char *archivoConf;
	int debugMode; // 1 = ON . 0 = OFF (default)
	int logPantalla; // 1 = ON . 0 = OFF (default)
} t_commandLineParameters;

//Conexiones
void iniciarConexiones();
void finalizar(int);
void esperarOrdenDeEjecucion();
tRespuestaCoordinador *recibirResultadoOperacion();

//Configuración
int cargarConfiguracion();
void limpiarConfiguracion();
void cargarArchivo(char *);
void cicloPrincipal();
int configValida(t_config *);
int procesarComandos(char **);
t_commandLineParameters parametrosEntrada;

//Proceso de Parseo
void ordenRecibida();
int leerLinea();
void enviarLineaOK();
void enviarOperacion();
void enviarEsiFinalizado();

#endif /* ESI_H_ */
