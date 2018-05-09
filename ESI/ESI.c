/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "ESI.h"
#include "..//shared/protocolo.h"
#include "..//shared/sockets.h"
#include "..//shared/serializar.h"
#include "..//shared/libgral.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

int main(int argn, char *argv[]) {

	cargarConfiguracion();
	iniciarConexiones();
	comenzarParseo();
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}

void iniciarConexiones() {
	int bytesEnviados;
	// TODO el log se está creando acá y en la carga de configuración. corregir esto.
	t_log *logESI = log_create("esi.log", "ESI",true, LOG_LEVEL_INFO);

	// TODO ¿por qué está hardcodeade la IP del servidor? corregir.
	configuracion->socketPlanificador = connectToServer("127.0.0.1",configuracion->puertoPlanificador, logESI);

	// TODO al pedo 2 variables iguales, se puede reusar la misma. corregir esto.
	tSolicitudESI* solicitud = malloc(sizeof(tSolicitudESI));
	tSolicitudESI* solicitud2 = malloc(sizeof(tSolicitudESI));

	solicitud->mensaje = malloc(100);
	strcpy(solicitud->mensaje, "HOLA SOY ESI!!!");
	tPaquete pkgHandshake;
	pkgHandshake.type = E_HANDSHAKE;

	tRespuestaPlanificador *respuesta = malloc(sizeof(tRespuestaPlanificador));
	// TODO ¿por qué se llama "respuestaCoordinador" pero es de tipo tRespuestaPlanificador?
	// corregir el nombre para que sea consistente
	tRespuestaPlanificador *respuestaCoordinador = malloc(sizeof(tRespuestaPlanificador));

	pkgHandshake.length = serializar(pkgHandshake.payload, "%c%s",pkgHandshake.type, solicitud->mensaje);

	puts("Se envia solicitud al Planificador");
	bytesEnviados = enviarPaquete(configuracion->socketPlanificador, &pkgHandshake, logESI,"Se envia solicitud de ejecucion");
	printf("Se envian %d bytes\n", bytesEnviados);

	//Recibo respuesta del Planificador
	char * sPayloadRespuestaHand = malloc(100);

	tMensaje tipoMensaje;

	int bytesRecibidos = recibirPaquete(configuracion->socketPlanificador, &tipoMensaje,&sPayloadRespuestaHand, logESI, "Hand Respuesta");
	printf("RECIBIDOS:%d\n", bytesRecibidos);

	respuesta->mensaje = malloc(100);
	char encabezado_mensaje;

	deserializar(sPayloadRespuestaHand, "%c%s", &encabezado_mensaje,respuesta->mensaje);
	printf("RESPUESTA: %s \n", respuesta->mensaje);

	if(strcmp(respuesta->mensaje,"OK")==0){
		//Abrir el script y comenzar a ejecutar sentencias
		puts("EJECUTANDO...");
		retardoSegundos(10);

		//Conexion al Coordinador
		// TODO corregir esta IP hardcodeada
		configuracion->socketCoordinador = connectToServer("127.0.0.1",configuracion->puertoCoordinador,logESI);

		solicitud2->mensaje = malloc(100);
		strcpy(solicitud2->mensaje, "HOLA SOY ESI!!!");
		tPaquete pkgHandshake2;
		pkgHandshake2.type = E_HANDSHAKE;

		pkgHandshake2.length = serializar(pkgHandshake2.payload, "%c%s",pkgHandshake2.type, solicitud2->mensaje);

		puts("Se envia solicitud de ejecucion al Coordinador");
		bytesEnviados = enviarPaquete(configuracion->socketCoordinador, &pkgHandshake2, logESI,"Se envia solicitud de ejecucion");
		printf("Se envian %d bytes\n", bytesEnviados);

		//RECIBIR RESPUESTA DEL COORDINADOR
		tMensaje tipoMensajeCoordinador;
		char * sPayloadRespuestaHandC = malloc(100);

		bytesRecibidos = recibirPaquete(configuracion->socketCoordinador, &tipoMensajeCoordinador,&sPayloadRespuestaHandC, logESI, "Hand Respuesta Coordinador");
		printf("RECIBIDOS:%d\n", bytesRecibidos);
		respuestaCoordinador->mensaje = malloc(100);
		char encabezadoMensaje;

		deserializar(sPayloadRespuestaHandC, "%c%s", &encabezadoMensaje,respuestaCoordinador->mensaje);

		printf("RESPUESTA COORDINADOR: %s \n", respuestaCoordinador->mensaje);

		// TODO en este procedimiento se hacen 10 malloc() y ningún free() !!! corregir esto.
	}
	//finalizar(0);
}

int comenzarParseo (int argc, char **argv){

	FILE * archivo;
	char * linea = NULL;
	size_t largo = 0;
	ssize_t lineaLeida;

	archivo = fopen(argv[1], "r");

	if (archivo == NULL){

		perror("Error al abrir el archivo: ");
		exit(EXIT_FAILURE);
	}

	while ((lineaLeida = getline(&linea, &largo, archivo)) != -1) {
	        t_esi_operacion lineaParseada = parse(linea);

	        if(lineaParseada.valido){

	            switch(lineaParseada.keyword){

	                case GET:
	                //Cada vez que haya un GET, no hay malloc, solo modifica el estado de bloqueo/desbloqueo
	                //desbloqueo de la tabla que debe tener el planificador. No se accede a ninguna instancia
	                //Cuando un ESI usa un GET sobre una clave, ningún ESI va a poder hacer GET de esa clave
	                //sin que el anterior ESI haga un STORE.
	                    printf("GET\tclave: <%s>\n", lineaParseada.argumentos.GET.clave);
	                    break;
	                case SET:
	                //Unica operación que altera valor de una instancia, previamente
	                //tiene que haber un GET que se apropie de una clave para ser alterada
	                    printf("SET\tclave: <%s>\tvalor: <%s>\n", lineaParseada.argumentos.SET.clave, lineaParseada.argumentos.SET.valor);
	                    break;
	                case STORE:
	                //Operación que libera una clave tomada. Trabaja con FIFO, libera de la
	                //tabla que tiene el planificador LA PRIMER CLAVE TOMADA.
	                    printf("STORE\tclave: <%s>\n", lineaParseada.argumentos.STORE.clave);
	                    break;
	                default:
	                    fprintf(stderr, "No pude interpretar <%s>\n", linea);
	                    exit(EXIT_FAILURE);
	            }

	            destruir_operacion(lineaParseada);
	        } else {
	            fprintf(stderr, "La linea <%s> no es valida\n", linea);
	            exit(EXIT_FAILURE);
	        }
	    }

	    fclose(archivo);
	    if (linea)
	        free(linea);

	    return EXIT_SUCCESS;
	}
}
