/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

int conectarACoordinador() {
	/* conecta con el coordinador y hace el handshake */

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
	enviarPaquete(configuracion.fdSocketCoordinador, &pkgHandshake, logger,"Enviando Handshake...");

	char * sPayloadRespuesta = (char *)malloc(50);
	char * mensajeRespuesta = (char *)malloc(50);
	tMensaje tipoMensaje;
	recibirPaquete(configuracion.fdSocketCoordinador, &tipoMensaje, &sPayloadRespuesta, logger, "Recibiendo respuesta de coordinador...");
	deserializar(sPayloadRespuesta, "%d;%d;%s", &(configuracion.cantidadEntradas), &(configuracion.tamanioEntrada), mensajeRespuesta);
	free(sPayloadRespuesta);

	if (!(configuracion.cantidadEntradas * configuracion.tamanioEntrada)) {
		log_error(logger,"Handshake no completado con el Coordinador: %s", mensajeRespuesta);
		mostrarTexto("ERROR: Handshake no completado con el Coordinador");
		mostrarTexto(mensajeRespuesta);
		free(mensajeRespuesta);
		return 0;
	}
	log_info(logger,"Handshake exitoso con el Coordinador: %s", mensajeRespuesta);
	free(mensajeRespuesta);

	return 1;
}
