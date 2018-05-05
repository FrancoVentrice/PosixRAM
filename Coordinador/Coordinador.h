#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

#define ALGORITMO_LSU 1
#define ALGORITMO_EL 2
#define ALGORITMO_KE 3

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


t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;
t_instancia * instanciaElegida;//es la instancia que elige el algoritmo de distribucion. cuando devuelva un ok, se agrega la clave en el diccionario, esta instancia como valor
t_dictionary * diccionarioClaves;//diccionario de claves e instancias que las poseen. key: clave, value: instancia que la tiene guardada
t_list * instancias;//lista de instancias disponibles
int punteroEL;//puntero usado en la distribucion Equitative Load
char * clave;//es la clave correspondiente a la operacion que se quiere ejecutar

int cargarConfiguracion();
void limpiarConfiguracion();
void finalizar(int);
int configValida(t_config *);
void escucharConexiones();
void elegirInstanciaSet();
void registrarClaveAgregadaAInstancia();
void elegirInstanciaLSU();
void elegirInstanciaEL();
void elegirInstanciaKE();

//instancia
void instanciaDestroyer(t_instancia *);
t_instancia * instanciaNew();

#endif /* COORDINADOR_H_ */
