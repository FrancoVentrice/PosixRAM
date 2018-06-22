/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

int conectarACoordinador(char ** clavesSincronizadas) {
	/* Conecta con el coordinador y hace el handshake. Recibe la lista de clves para sincronizar.
	 * No sale de la función hasta que recibe la respuesta, por lo que se podría quedar acá. */

	int bytesEnviados;
	int bytesLeidos;

	log_info(logger,"Conectando con el Coordinador (IP: %s Puerto: %d)...", configuracion.ipCoordinador,configuracion.puertoCoordinador);
	configuracion.fdSocketCoordinador = connectToServer(configuracion.ipCoordinador,configuracion.puertoCoordinador, logger);
	if (configuracion.fdSocketCoordinador < 0) {
		log_error(logger,"No se pudo conectar con el Coordinador.");
		mostrarTexto("ERROR: No se pudo conectar con el Coordinador.");
		return 0;
	}
	log_info(logger,"Conexión exitosa con el Coordinador.");

	// handshake con coordinador
	log_info(logger,"Realizando handshake con el Coordinador.");
	tPaquete pkgHandshake;
	pkgHandshake.type = I_HANDSHAKE;
	pkgHandshake.length = serializar(pkgHandshake.payload, "%s",configuracion.nombreDeInstancia);
	bytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgHandshake, logger,"Enviando Handshake...");
	log_debug(logger,"Se enviaron %d bytes", bytesEnviados);

	char * sPayloadRespuesta = (char *)malloc(MAX_BUFFER);
	memset(sPayloadRespuesta,0,MAX_BUFFER);

	tMensaje tipoMensaje;
	bytesLeidos = recibirPaquete(configuracion.fdSocketCoordinador, &tipoMensaje, &sPayloadRespuesta, logger, "Recibiendo respuesta de coordinador...");
	//  El tipoMensaje es anecdótico en este punto, porque sabemos que va a ser C_HANDSHAKE
	log_debug(logger,"Mensaje recibido del coordinador tipo %d (Handshake = %d) de %d bytes.",tipoMensaje,C_HANDSHAKE, bytesLeidos);

	char * mensajeRespuesta = (char *)malloc(60);
	memset(mensajeRespuesta,0,60);
	*clavesSincronizadas = realloc(*clavesSincronizadas, bytesLeidos);
	memset(*clavesSincronizadas,0,bytesLeidos);

	deserializar(sPayloadRespuesta, "%d%d%s%s", &(configuracion.cantidadEntradas), &(configuracion.tamanioEntrada), mensajeRespuesta, *clavesSincronizadas);
	free(sPayloadRespuesta);

	if (!(configuracion.cantidadEntradas * configuracion.tamanioEntrada)) {
		log_error(logger,"Handshake no completado con el Coordinador: %s", mensajeRespuesta);
		mostrarTexto("ERROR: Handshake no completado con el Coordinador");
		mostrarTexto(mensajeRespuesta);
		free(mensajeRespuesta);
		free(*clavesSincronizadas);
		return 0;
	}
	log_info(logger,"Handshake exitoso con el Coordinador: %s", mensajeRespuesta);
	free(mensajeRespuesta);

	return 1;
}
