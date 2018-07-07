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

	char * sPayloadRespuesta;

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

int atenderEstadoClave(char * sPayloadRespuesta) {
	/* Procesa el pedido de Estado Clave devolviendo el valor de la misma */

	char * claveRecibida;
	char * valorDeEntrada;
	int iBytesEnviados;
	claveRecibida = (char *)malloc(MAX_LONG_CLAVE);

	deserializar(sPayloadRespuesta, "%s", claveRecibida);

	log_info(logger,"Buscando valor para la clave: %s",claveRecibida);

	tPaquete pkgResultadoEstadoClave;

	if(existeClave(claveRecibida)) {
		valorDeEntrada = valorDeEntradaPorClave(claveRecibida);
		log_info(logger,"Valor encontrado: %s",valorDeEntrada);

		pkgResultadoEstadoClave.type = I_ESTADO_CLAVE;
		pkgResultadoEstadoClave.length = serializar(pkgResultadoEstadoClave.payload, "%s", valorDeEntrada);

		free(valorDeEntrada);
	}
	else {
		log_error(logger,"La clave solicitada no está en la tabla de entradas.");

		pkgResultadoEstadoClave.type = I_RESULTADO_ERROR;
		pkgResultadoEstadoClave.length = serializar(pkgResultadoEstadoClave.payload, "%s", "No existe la clave solicitada");
	}

	iBytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgResultadoEstadoClave, logger, "Se envía la respuesta al Coordinador.");
	free(claveRecibida);
	return iBytesEnviados;
}

int atenderStoreClave(char * sPayloadRespuesta) {
	/* Realiza el store a disco de la clave con su valor, y envía la respuesta */

	char * claveRecibida;
	int iBytesEnviados;
	claveRecibida = (char *)malloc(MAX_LONG_CLAVE);

	deserializar(sPayloadRespuesta, "%s", claveRecibida);

	tPaquete pkgResultadoStore;

	if(existeClave(claveRecibida)) {
		log_info(logger,"Realizando store de la clave: %s",claveRecibida);

		storeClave(indiceClave(claveRecibida));

		pkgResultadoStore.type = I_RESULTADO_STORE;
		pkgResultadoStore.length = serializar(pkgResultadoStore.payload, "", NULL);
	}
	else {
		log_error(logger,"La clave solicitada no está en la tabla de entradas.");

		pkgResultadoStore.type = I_RESULTADO_ERROR;
		pkgResultadoStore.length = serializar(pkgResultadoStore.payload, "%s", "No existe la clave solicitada");
	}

	iBytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgResultadoStore, logger, "Se envía resultado del store al Coordinador");
	free(claveRecibida);
	return iBytesEnviados;
}

int atenderEjecutarCompactacion() {
	/* Ejecuta la compactación de la instancia y responde al coordinador cuando finaliza */

	log_info(logger,"Iniciando proceso de compactación de la Instancia.");

	realizarCompactacion();

	tPaquete pkgResultadoCompactacion;

	pkgResultadoCompactacion.type = I_COMPACTACION_TERMINADA;
	pkgResultadoCompactacion.length = serializar(pkgResultadoCompactacion.payload, "", NULL);

	return enviarPaquete(configuracion.fdSocketCoordinador, &pkgResultadoCompactacion, logger, "Se envía fin de compactación al Coordinador");
}

int atenderSetClaveValor(char * sPayloadRespuesta) {

	int iBytesEnviados;
	char * claveRecibida = (char *)calloc(1, MAX_LONG_CLAVE);
	char * valorRecibido = (char *)calloc(1, MAX_BUFFER);

	deserializar(sPayloadRespuesta, "%s%s", claveRecibida, valorRecibido);

	// *************************************************************************
	// ToDo: lo que sigue es la respuesta
	// *************************************************************************
	tPaquete pkgResultadoSet;
	t_respuestaSet respuestaSet;

	strcpy(respuestaSet.claveReemplazada,claveRecibida);
	respuestaSet.compactacionRequerida = 0;
	//tablaDeEntradas[indice].ultimaInstruccion = configuracion.instruccionesProcesadas;

	pkgResultadoSet.type = I_RESULTADO_SET;
	pkgResultadoSet.length = serializar(pkgResultadoSet.payload, "%d%s%c", entradasDisponibles(), respuestaSet.claveReemplazada, respuestaSet.compactacionRequerida);

	iBytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgResultadoSet, logger, "Se envía resultado del set al Coordinador");
	// *************************************************************************
	// *************************************************************************

	free(claveRecibida);
	free(valorRecibido);

	return iBytesEnviados;

}
