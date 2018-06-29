#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <parsi/parser.h>
#include <pthread.h>
#include "../shared/sockets.h"
#include "../shared/serializar.h"
#include "../shared/protocolo.h"
#include "../shared/libgral.h"

#define ALGORITMO_LSU 1
#define ALGORITMO_EL 2
#define ALGORITMO_KE 3
#define PATH_LOG "..//Coordinador/log/logOperaciones.txt"
#define OPERACION_GET 1
#define OPERACION_SET 2
#define OPERACION_STORE 3

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

typedef struct {
	char *nombre;
	int socket;
	int cantidadDeEntradasDisponibles;
} t_instancia;

typedef struct{
	int operacion;
	char* clave;
	char* valor;
	int remitente;
} t_operacionESI;

t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;
t_log * logDeOperaciones;
t_instancia * instanciaElegida;//es la instancia que elige el algoritmo de distribucion. cuando devuelva un ok, se agrega la clave en el diccionario, esta instancia como valor
t_dictionary * diccionarioClaves;//diccionario de claves e instancias que las poseen. key: clave, value: instancia que la tiene guardada
t_list * instancias;//lista de instancias disponibles
int punteroEL;//puntero usado en la distribucion Equitative Load
t_operacionESI * operacion;

fd_set setSockets;
int socketPlanificador;
int socketEscucha;
int maxSock;
pthread_t hiloEscucha;

int cargarConfiguracion();
void inicializarSockets();
void levantarHiloEscucha();
void escucharConexiones();
void cicloPrincipal();
void consultarPlanificador();
void limpiarConfiguracion();
void finalizar(int);
int configValida(t_config *);
t_instancia * elegirInstancia(bool consulta);
void registrarClaveAgregadaAInstancia();
t_instancia * elegirInstanciaLSU();
t_instancia * elegirInstanciaEL(bool consulta);
t_instancia * elegirInstanciaKE();
void escribirLogDeOperaciones();
void informarResultadoOperacionOk();
void informarResultadoOperacionBloqueado();
void informarResultadoOperacionError();
char * recibirRespuestaConsulta();
void accionarFrenteAConsulta(char *);
void evaluarEstadoDeClave(char *);
void enviarOperacionAInstancia(void);
void recibirOperacionDeInstancia(void);
char * buscarClavesPorInstancia(char *nombreInstancia);
void ejecutarCompactacion();

//instancia
void instanciaDestroyer(t_instancia *);
int existeInstanciaConectadaConMismoNombre(char *);
int existeInstanciaDesconectadaConMismoNombre(char *);
int validarDesconexionDeInstancia(int);
void nuevaInstanciaConectada(char *, int);
void instanciaReconectada(char *, int);

#endif /* COORDINADOR_H_ */
