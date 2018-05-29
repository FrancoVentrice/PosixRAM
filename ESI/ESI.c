/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "ESI.h"
#include "..//shared/protocolo.h"
#include "..//shared/sockets.h"
#include "..//shared/serializar.h"
#include "..//shared/libgral.h"
#include "..//shared/logExtra.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	cargarArchivo(argv[1]);
	iniciarConexiones();
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}

void iniciarConexiones() {
	int bytesEnviados;
	int bytesRecibidos;
	/*
	 *
	 */
	tSolicitudESI* solicitud = malloc(sizeof(tSolicitudESI));
	tRespuestaPlanificador *respuestaCoordinador = malloc(
			sizeof(tRespuestaCoordinador));
	tRespuestaPlanificador *respuestaPlanificador = malloc(
			sizeof(tRespuestaPlanificador));

	char* respuestaOperacion = malloc(10);

	//COnexion al Coordinador
	configuracion->socketCoordinador = connectToServer(
			configuracion->ipCoordinador, configuracion->puertoCoordinador,
			logger);

	solicitud->mensaje = malloc(100);
	strcpy(solicitud->mensaje, "HOLA SOY ESI!!!");
	tPaquete pkgHandshake2;
	pkgHandshake2.type = E_HANDSHAKE;

	pkgHandshake2.length = serializar(pkgHandshake2.payload, "%c%s",
			pkgHandshake2.type, solicitud->mensaje);

	log_info(logger, "Se envia solicitud de ejecucion al Coordinador");
	bytesEnviados = enviarPaquete(configuracion->socketCoordinador,
			&pkgHandshake2, logger, "Se envia solicitud de ejecucion");
	log_info(logger, "Se envian %d bytes", bytesEnviados);

//

//RECIBIR RESPUESTA DEL COORDINADOR
	tMensaje tipoMensajeCoordinador;
	char * sPayloadRespuestaHandC = malloc(100);

	bytesRecibidos = recibirPaquete(configuracion->socketCoordinador,
			&tipoMensajeCoordinador, &sPayloadRespuestaHandC, logger,
			"Hand Respuesta Coordinador");
	log_info(logger, "RECIBIDOS:%d", bytesRecibidos);
	respuestaCoordinador->mensaje = malloc(100);
	char encabezadoMensaje;

	deserializar(sPayloadRespuestaHandC, "%c%s", &encabezadoMensaje,
			respuestaCoordinador->mensaje);

	log_info(logger, "RESPUESTA COORDINADOR: %s",
			respuestaCoordinador->mensaje);

	//CONEXION AL PLANIFICADOR
	configuracion->socketPlanificador = connectToServer(
			configuracion->ipPlanificador, configuracion->puertoPlanificador,
			logger);

	solicitud->mensaje = malloc(100);
	strcpy(solicitud->mensaje, "HOLA SOY ESI!!!");
	tPaquete pkgHandshake;
	pkgHandshake.type = E_HANDSHAKE;

	tRespuestaPlanificador *respuesta = malloc(sizeof(tRespuestaPlanificador));

	pkgHandshake.length = serializar(pkgHandshake.payload, "%c%s",
			pkgHandshake.type, solicitud->mensaje);

	log_info(logger, "Se envia solicitud al Planificador");
	bytesEnviados = enviarPaquete(configuracion->socketPlanificador,
			&pkgHandshake, logger, "Se envia solicitud de ejecucion");
	log_info(logger, "Se envian %d bytes", bytesEnviados);
	//Recibo respuesta del Planificador
	char * sPayloadRespuestaHand = malloc(100);

	tMensaje tipoMensaje;

	bytesRecibidos = recibirPaquete(configuracion->socketPlanificador,
			&tipoMensaje, &sPayloadRespuestaHand, logger, "Hand Respuesta");
	log_info(logger, "RECIBIDOS:%d", bytesRecibidos);

	respuesta->mensaje = malloc(100);
	char encabezado_mensaje;

	deserializar(sPayloadRespuestaHand, "%c%s", &encabezado_mensaje,
			respuesta->mensaje);
	log_info(logger, "RESPUESTA: %s", respuesta->mensaje);

	//Recibir respuesta por parte del Planificador para ejecutar el script
	tMensaje tipoMensajePlanificador;
	char * sPayloadRespuestaPlanificador = malloc(100);

	log_info(logger, "Esperando orden de ejecucion...");
	bytesRecibidos = recibirPaquete(configuracion->socketPlanificador,
			&tipoMensajePlanificador, &sPayloadRespuestaPlanificador, logger,
			"Respuesta a la ejecucion");
	log_info(logger, "RECIBIDOS:%d", bytesRecibidos);
	respuestaPlanificador->mensaje = malloc(100);
	char encabezadoMensajePlanificador;

	deserializar(sPayloadRespuestaPlanificador, "%c%s",
			&encabezadoMensajePlanificador, respuestaPlanificador->mensaje);

	log_info(logger, "RESPUESTA PLANIFICADOR: %s",
			respuestaPlanificador->mensaje);

	//ENVIO AL COORDINADOR LA INSTRUCCION A EJECUTAR

	ordenRecibida();

	/*recibirResultadoOperacion(bytesRecibidos, respuestaCoordinador);

	if (strcmp(respuestaCoordinador, "OK")) { // ACA DEBERIA RECIBIR EL OK DEL PLANIFIFCADOR
											  //PARA SEGUIR EJECUTANDO LA SIGUIENTE LINEA.
											  //ESTO ES SOLO UNA PRUEBA
		ordenRecibida();
	}*/

	// TODO en este procedimiento se hacen 10 malloc() y ningún free() !!! corregir estosolicitud
	/*free(solicitud);
	 free(respuestaCoordinador);
	 free(sPayloadRespuestaHandC);//
	 free(sPayloadRespuestaPlanificador);
	 free(respuestaCoordinador);
	 free(sPayloadRespuestaHand);
	 free(sPayloadRespuestaHandC);*/

}
void recibirResultadoOperacion(int recibidos,
		tRespuestaCoordinador* respuestaCoordinador) {
	tMensaje tipoMensajeCoordinador;
	char * sPayloadRespuestaCoordinador = malloc(100);

	log_info(logger, "Esperando respuesta de la operacion...");
	recibidos = recibirPaquete(configuracion->socketCoordinador,
			&tipoMensajeCoordinador, &sPayloadRespuestaCoordinador, logger,
			"Respuesta a la ejecucion");
	log_info(logger, "RECIBIDOS:%d", recibidos);
	respuestaCoordinador->mensaje = malloc(100);

	deserializar(sPayloadRespuestaCoordinador, "%s",
			respuestaCoordinador->mensaje);

	log_info(logger, "RESPUESTA OPERACION DEL COORDINADOR : %s",
			respuestaCoordinador->mensaje);

}
void cargarArchivo(char *path) {
	archivo = fopen(path, "r");
	if (archivo == NULL) {
		log_error(logger, "Error al abrir el archivo: %s", path);
		finalizar(EXIT_FAILURE);
	}
}

//este metodo se ejecuta cuando se recibe la orden de lectura del planificador
//evalua si hay que mandar la misma linea otra vez, o una nueva
//tambien evalua si el programa finalizo
void ordenRecibida() {
	if (lecturaRechazada) {
		enviarOperacion();
	} else {
		if (leerLinea() < 0) {
			enviarEsiFinalizado(); //ENVIO FIN DE LECTURA AL PLANIFICADOR
		} else {
			enviarOperacion();//ENVIO AL COORDINADOR LA SENTENCIA
			enviarLineaOK(); //ENVIO OK AL PLANIFICADOR
		}
	}
}

int leerLinea() {
	ssize_t lectura;
	free(operacion->clave);
	free(operacion->valor);
	operacion->clave = NULL;
	operacion->valor = NULL;
	if ((lectura = getline(&lineptr, &n, archivo)) != -1) {

		t_esi_operacion lineaParseada = parse(lineptr);

		if (lineaParseada.valido) {
			switch (lineaParseada.keyword) {
			case GET:
				operacion->operacion = OPERACION_GET;
				operacion->clave = malloc(
						string_length(lineaParseada.argumentos.GET.clave));
				strcpy(operacion->clave, lineaParseada.argumentos.GET.clave);
				break;
			case SET:
				operacion->operacion = OPERACION_SET;
				operacion->clave = malloc(
						string_length(lineaParseada.argumentos.SET.clave));
				strcpy(operacion->clave, lineaParseada.argumentos.SET.clave);
				operacion->valor = malloc(
						string_length(lineaParseada.argumentos.SET.valor));
				strcpy(operacion->valor, lineaParseada.argumentos.SET.valor);

				break;
			case STORE:
				operacion->operacion = OPERACION_STORE;
				operacion->clave = malloc(
						string_length(lineaParseada.argumentos.STORE.clave));
				strcpy(operacion->clave, lineaParseada.argumentos.STORE.clave);

				break;
			default:
				log_error(logger, "No pude interpretar <%s>\n", lineptr);
				finalizar(EXIT_FAILURE);

			}

			destruir_operacion(lineaParseada);

			log_info(logger, "\noperacion: %d \n clave: %s \n valor: %s\n",
					operacion->operacion, operacion->clave, operacion->valor);
		} else {
			log_error(logger, "La linea <%s> no es valida\n", lineptr);
			finalizar(EXIT_FAILURE);
		}
	} else {
		return -1;
	}
	return 0;
}
void enviarLineaOK(){
	tPaquete pkgLineaOk;
	int bytesEnviados;
	char* lineaOk=malloc(5);
	strcpy(lineaOk,"OK");
	pkgLineaOk.type = E_LINEA_OK;  //

	pkgLineaOk.length = serializar(pkgLineaOk.payload, "%s",
			lineaOk);

	log_info(logger, "Se envia OK al Planificador");
	bytesEnviados = enviarPaquete(configuracion->socketPlanificador,
			&pkgLineaOk, logger,
			"Se envia OK al Planificador");
	log_info(logger, "Se envian %d bytes", bytesEnviados);


}
void enviarOperacion() {
	lecturaRechazada = false;
	//en este metodo se envia la operacion leida, la cual esta guardada en
	//la variable global "operacion"
	tPaquete pkgSentencia;
	int bytesEnviados;

	if (operacion->operacion == OPERACION_GET) {
		pkgSentencia.type = E_SENTENCIA_GET;  //

		pkgSentencia.length = serializar(pkgSentencia.payload, "%s",
				operacion->clave);

		log_info(logger, "Se envia la instruccion GET al coordinador");
		bytesEnviados = enviarPaquete(configuracion->socketCoordinador,
				&pkgSentencia, logger,
				"Se envia la instruccion GET al coordinador");
		log_info(logger, "Se envian %d bytes", bytesEnviados);

	} else if (operacion->operacion == OPERACION_SET) {
		pkgSentencia.type = E_SENTENCIA_SET;  //

		pkgSentencia.length = serializar(pkgSentencia.payload, "%s%s",
				operacion->clave, operacion->valor);

		log_info(logger, "Se envia la instruccion SET al coordinador");
		bytesEnviados = enviarPaquete(configuracion->socketCoordinador,
				&pkgSentencia, logger,
				"Se envia la instruccion SET al coordinador");
		log_info(logger, "Se envian %d bytes", bytesEnviados);

	} else if (operacion->operacion == OPERACION_STORE) {
		pkgSentencia.type = E_SENTENCIA_STORE;  //

		pkgSentencia.length = serializar(pkgSentencia.payload, "%s",
				operacion->clave);

		log_info(logger, "Se envia la instruccion STORE al coordinador");
		bytesEnviados = enviarPaquete(configuracion->socketCoordinador,
				&pkgSentencia, logger,
				"Se envia la instruccion STORE al coordinador");
		log_info(logger, "Se envian %d bytes", bytesEnviados);

	}

}

void enviarEsiFinalizado() {
	//en este metodo se informa al planificador que ya no hay lineas para leer
	//y el ESI finalizo su ejecucion
	tPaquete pkgSentencia;
	int bytesEnviados;
	char* mensajeOk=malloc(5);
	strcpy(mensajeOk,"ESI FINALIZADO");

	pkgSentencia.type = E_ESI_FINALIZADO;  //

	pkgSentencia.length = serializar(pkgSentencia.payload, "%s",
			mensajeOk);

	log_info(logger, "Se envia respuesta Finalizado");
	bytesEnviados = enviarPaquete(configuracion->socketCoordinador,
			&pkgSentencia, logger,
			"Se envia respuesta Finalizado");
	log_info(logger, "Se envian %d bytes", bytesEnviados);

}
