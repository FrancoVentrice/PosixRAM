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
	iniciarConexiones(argn,argv);
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}

void iniciarConexiones(int argn,char *argv[]) {
	int bytesEnviados;
	// TODO el log se está creando acá y en la carga de configuración. corregir esto.

	// TODO ¿por qué está hardcodeade la IP del servidor? corregir.
	configuracion->socketPlanificador = connectToServer(configuracion->ipPlanificador,configuracion->puertoPlanificador, logger);

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

	log_info(logger,"Se envia solicitud al Planificador");
	bytesEnviados = enviarPaquete(configuracion->socketPlanificador, &pkgHandshake, logger,"Se envia solicitud de ejecucion");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

	//Recibo respuesta del Planificador
	char * sPayloadRespuestaHand = malloc(100);

	tMensaje tipoMensaje;

	int bytesRecibidos = recibirPaquete(configuracion->socketPlanificador, &tipoMensaje,&sPayloadRespuestaHand, logger, "Hand Respuesta");
	log_info(logger,"RECIBIDOS:%d\n", bytesRecibidos);

	respuesta->mensaje = malloc(100);
	char encabezado_mensaje;

	deserializar(sPayloadRespuestaHand, "%c%s", &encabezado_mensaje,respuesta->mensaje);
	log_info(logger,"RESPUESTA: %s \n", respuesta->mensaje);

	//COnexion al Coordinador
	configuracion->socketCoordinador = connectToServer(configuracion->ipCoordinador,
			configuracion->puertoCoordinador, logger);

	solicitud2->mensaje = malloc(100);
	strcpy(solicitud2->mensaje, "HOLA SOY ESI!!!");
	tPaquete pkgHandshake2;
	pkgHandshake2.type = E_HANDSHAKE;

	pkgHandshake2.length = serializar(pkgHandshake2.payload, "%c%s",
			pkgHandshake2.type, solicitud2->mensaje);

	log_info(logger,"Se envia solicitud de ejecucion al Coordinador");
	bytesEnviados = enviarPaquete(configuracion->socketCoordinador, &pkgHandshake2,
			logger, "Se envia solicitud de ejecucion");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);



			//RECIBIR RESPUESTA DEL COORDINADOR
	tMensaje tipoMensajeCoordinador;
	char * sPayloadRespuestaHandC = malloc(100);

	bytesRecibidos = recibirPaquete(configuracion->socketCoordinador,
			&tipoMensajeCoordinador, &sPayloadRespuestaHandC, logger,
			"Hand Respuesta Coordinador");
	log_info(logger,"RECIBIDOS:%d\n", bytesRecibidos);
	respuestaCoordinador->mensaje = malloc(100);
	char encabezadoMensaje;

	deserializar(sPayloadRespuestaHandC, "%c%s", &encabezadoMensaje,
			respuestaCoordinador->mensaje);

	log_info(logger,"RESPUESTA COORDINADOR: %s \n", respuestaCoordinador->mensaje);


	//Recibir la linea a ejecutar por parte del Planificador

	// TODO en este procedimiento se hacen 10 malloc() y ningún free() !!! corregir esto.

	//finalizar(0);
}

void enviarOperacion(t_esi_operacion lineaParseada){
		}

int comenzarParseo (int argc, char **argv){

	FILE * archivo;
	char * linea = NULL;
	size_t largo = 0;
	ssize_t lineaLeida;
	int cantLineasLeidas=0;

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
	                    cantLineasLeidas++;
						enviarOperacion(lineaParseada);
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

