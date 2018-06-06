#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <signal.h>
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
#define OPERACION_GET 1
#define OPERACION_SET 2
#define OPERACION_STORE 3

typedef struct {
	int puerto;
	int algoritmoPlanificacion;
	int estimacionInicial;
	char* ipCoordinador;
	int puertoCoordinador;
	float alfa;
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
}tRespuestaCoordinador;

typedef struct{
	char * mensaje;
}tRespuesta;

typedef struct {
	int instruccion;
	char * primerParametro;
	char * segundoParametro;
} t_instruccion_consola;

typedef struct{
	int operacion;
	char* clave;
	char* valor;
} tConsultaCoordinador;

typedef struct {
	char * id;
	float estimacion; //se puede usar con los dos algoritmos
	int rafagaAnterior; //necesario para SJF. Se suma uno cada vez que el ESI ejecuta correctamente una sentencia
	float estimacionAnterior;//necesario para SJF
	int socket; //socket que se esta usando para la comunicacion con el ESI en particular
	bool bloqueado;
	int responseRatio;
	int instanteLlegadaAListos;
	t_list *clavesTomadas;
} t_esi;


fd_set setSockets;
int socketEscucha;
int socketCoordinador;
int maxSock;

t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;
pthread_t hiloConsola;
pthread_t hiloEscuchaESIs;
t_list * bufferConsola; //buffer de instrucciones a ejecutar cuando se complete una tarea atomica
int tiempoTotalEjecucion;
bool ejecutando; //se usa para saber si seguir ejecutando operaciones. se modifica desde consola
bool vivo; //se usa para repetir el ciclo de trabajo y eventualmente finalizar el proceso
bool aptoEjecucion; //esta en true siempre y cuando haya algun ESI apto de ejecucion
bool planificacionNecesaria; //se usa para saber si un evento gatillo una necesidad de planificar
pthread_mutex_t mutexColaDeListos;
tConsultaCoordinador* consultaCoordinador;
int nId;


t_esi * esiEnEjecucion; //vendria a ser la "cola" de ejecucion
t_list * colaDeListos; //lista de t_esi. es la cola de esis listos
t_list * colaDeFinalizados; //lista de t_esi. es la cola de esis finalizados
t_dictionary * diccionarioBloqueados; //diccionario de listas de esis. key: clave, value: cola de bloqueados por clave
t_dictionary * diccionarioClavesTomadas; //diccionario de esis por clave tomada. key: clave, value: esi que la tomo

void cicloPrincipal();
void cicloDeSentencia();
void enviarOrdenDeEjecucion();
int cargarConfiguracion();
int configValida(t_config *);
void limpiarConfiguracion();
void finalizar(int);
void levantarConsola();
void consola();
void realizarHandshakeCoordinador();
void escucharConexionesESIs();
void agregarEsiAColaDeListos(t_esi *);
void planificar();
void bloquearESIConClave(t_esi *, char *);
void esiTomaClave(t_esi *, char *);
void bloquearClaveSola(char *);
void liberarClave(char *);
void liberarPrimerProcesoBloqueado(char *);
void ejecutarComandosConsola();
void bloquearEsiPorConsola(char *, char *);
void abortarEsiPorId(char *);
t_esi * encontrarEsiPorId(t_list *, char *);
int getIndexDeEsi(t_list *, t_esi *);
void analizarDeadlock();
void recibirConsultaOperacion(tMensaje, char *);

//metodos de ESI
void liberarClavesDeEsi(t_esi *);
t_esi *esiNew();
void esiDestroyer(t_esi *);
void esiListDestroyer(t_list *);
bool evaluarBloqueoDeEsi(t_esi *);
bool evaluarBloqueoDeClave(char *);
void esiRemoverClaveTomada(t_esi *, char *);
t_esi *buscarEsiNoBloqueadoPorId(char *);
void clavesTomadasDestroyer(char *);
void finalizarEsiEnEjecucion();
void estimar(t_esi *);
void sentenciaEjecutada();
void esiFinalizado();

//metodos de HRRN
int planificarHRRN();
int calcularTiempoEspera(t_esi *);

//metodos de SJF
int planificarSJF();

//metodos de la consola
void pause();
void play();
void lock(char *);
void unlock(char *);
void list(char *);
void killEsi(char *);
void status(char *);
void deadlock();
void exitPlanificador();
void instruccionDestroyer(t_instruccion_consola *);
void newInstruccion(int , char *, char *);

#endif /* PLANIFICADOR_H_ */
