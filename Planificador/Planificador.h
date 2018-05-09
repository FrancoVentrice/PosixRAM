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

#define ALGORITMO_SJF_CD 1
#define ALGORITMO_SJF_SD 2
#define ALGORITMO_HRRN 3
#define INSTRUCCION_BLOQUEAR 11
#define INSTRUCCION_DESBLOQUEAR 12
#define INSTRUCCION_TERMINAR 13
#define INSTRUCCION_DEADLOCK 14

typedef struct {
	int puerto;
	int algoritmoPlanificacion;
	int estimacionInicial;
	char* ipCoordinador;
	int puertoCoordinador;
	int alfa;
	char* clavesInicialmenteBloqueadas;
} t_configuracion;

typedef struct{
	char * mensaje;
} tSolicitudESI;

typedef struct{
	char * mensaje;
} tSolicitudPlanificador;

typedef struct{
	char * mensaje;
}tRespuesta;

typedef struct {
	int instruccion;
	char * primerParametro;
	char * segundoParametro;
} t_instruccion_consola;

typedef struct {
	char * id;
	float estimacion; //se puede usar con los dos algoritmos
	int rafagaAnterior; //necesario para SJF. Se suma uno cada vez que el ESI ejecuta correctamente una sentencia
	float estimacionAnterior;//necesario para SJF
	int socket; //socket que se esta usando para la comunicacion con el ESI en particular
	bool bloqueado;
	bool bloqueadoPorConsola; //solo sirve para saber si se puede desbloquear por consola
	int responseRatio;
	int instanteLlegadaAListos;
	t_list *clavesTomadas;
} t_esi;

t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;
pthread_t hiloConsola;
t_list * bufferConsola; //buffer de instrucciones a ejecutar cuando se complete una tarea atomica
int tiempoTotalEjecucion;
float alfa;
bool ejecutando; //se usa para saber si seguir ejecutando operaciones. se modifica desde consola


t_esi * esiEnEjecucion; //vendria a ser la "cola" de ejecucion
t_list * colaDeListos; //lista de t_esi. es la cola de esis listos
t_list * colaDeFinalizados; //lista de t_esi. es la cola de esis finalizados
t_dictionary * diccionarioBloqueados; //diccionario de listas de esis. key: clave, value: cola de bloqueados por clave
t_dictionary * diccionarioClavesTomadas; //diccionario de esis por clave tomada. key: clave, value: esi que la tomo

int cargarConfiguracion();
int configValida(t_config *);
void limpiarConfiguracion();
void finalizar(int);
void levantarConsola();
void consola();
void escucharESIs();
void bloquearESIConClave(t_esi *, char *);
void esiTomaClave(t_esi *, char *);
void bloquearClaveSola(char *);
void liberarClave(char *);
void planificar();
void ejecutarComandosConsola();
void bloquearEsiPorConsola(char *, char *);

//metodos de ESI
t_esi *esiNew();
void esiDestroyer(t_esi *);
void esiListDestroyer(t_list *);
bool evaluarBloqueoDeEsi(t_esi *);
bool evaluarBloqueoDeClave(char *);
void esiRemoverClaveTomada(t_esi *, char *);
t_esi *buscarEsiNoBloqueadoPorId(char *);
void clavesTomadasDestroyer(char *);
void finalizarEsiEnEjecucion();

//metodos de HRRN
void estimarHRRN();
int calcularTiempoRespuesta(t_esi *);

//metodos de SJF
void estimarSJF();
void sentenciaEjecutadaCorrectamenteSJF();

//metodos de la consola
void pause();
void play();
void lock(char *);
void unlock(char *);
void list(char *);
void kill(char *);
void status(char *);
void deadlock();
void instruccionDestroyer(t_instruccion_consola *);
void newInstruccion(int , char *, char *);

#endif /* PLANIFICADOR_H_ */
