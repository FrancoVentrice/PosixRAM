/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "ESI.h"

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	cargarArchivo(argv[1]);
	iniciarConexiones();
	cicloPrincipal();
}

void cicloPrincipal() {
	while (ejecucion) {
		esperarOrdenDeEjecucion();
		ordenRecibida();
		tRespuestaCoordinador *resultadoDeEjecucion = recibirResultadoOperacion();

		if (strcmp(resultadoDeEjecucion->mensaje, "OK") == 0) {
			log_info(logger, "lectura aprobada!");
		} else if (strcmp(resultadoDeEjecucion->mensaje, "BLOQUEADO") == 0) {
			log_info(logger, "me bloquearon :(");
			lecturaRechazada = true;
		} else if (strcmp(resultadoDeEjecucion->mensaje, "ERROR") == 0) {
			log_info(logger, "adios mundo crueeel!! (voy a esperar que el plani me lo confirme)");
		}
	}
	finalizar(EXIT_FAILURE);
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}

void iniciarConexiones() {
	//Armo el paquete de handshake
	tPaquete pkgHandshakeESI;
	pkgHandshakeESI.type = E_HANDSHAKE;
	pkgHandshakeESI.length = serializar(pkgHandshakeESI.payload, "", NULL);

	//Conexion al Coordinador
	configuracion->socketCoordinador = connectToServer(configuracion->ipCoordinador, configuracion->puertoCoordinador, logger);
	int bytesEnviadosACoordinador = enviarPaquete(configuracion->socketCoordinador, &pkgHandshakeESI, logger, "Se envia solicitud de ejecucion");
	log_info(logger, "Enviado handshake a Coordinador: %d bytes", bytesEnviadosACoordinador);

	//Recibo respuesta de Coordinador
	tMensaje tipoMensajeCoordinador;
	char *bufferHSCoordinador;
	int bytesRecibidosDeCoordinador = recibirPaquete(configuracion->socketCoordinador, &tipoMensajeCoordinador, &bufferHSCoordinador, logger, "HS Respuesta Coordinador");
	log_info(logger, "Recibido handshake de Coordinador: %d bytes", bytesRecibidosDeCoordinador);
	if (tipoMensajeCoordinador == C_HANDSHAKE) {
		log_info(logger, "Hanshake con Coordinador OK");
	}

	//Conexion al Planificador
	configuracion->socketPlanificador = connectToServer(configuracion->ipPlanificador, configuracion->puertoPlanificador, logger);
	tMensaje tipoMensajePlanificador;
	char *bufferHSPlanificador;
	int bytesRecibidosDePlanificador = recibirPaquete(configuracion->socketPlanificador, &tipoMensajePlanificador, &bufferHSPlanificador, logger, "HS Respuesta Planificador");
	log_info(logger, "Recibido handshake de Planificador: %d bytes", bytesRecibidosDePlanificador);
	if (tipoMensajePlanificador == P_HANDSHAKE) {
		log_info(logger, "Hanshake con Planificador OK");
	}
}

void esperarOrdenDeEjecucion() {
	//Recibir orden por parte del Planificador para ejecutar el script
	tMensaje tipoMensajePlanificador;
	char *buffer;

	log_info(logger, "Esperando orden de ejecucion...");
	int bytesRecibidos = recibirPaquete(configuracion->socketPlanificador, &tipoMensajePlanificador, &buffer, logger, "Orden de ejecucion");

	log_info(logger, "tipo mensaje recibido: %d", tipoMensajePlanificador);

	if (tipoMensajePlanificador == P_EJECUTAR_LINEA) {
		log_info(logger, "Orden de lectura recibida en %d bytes", bytesRecibidos);
	} else if (tipoMensajePlanificador == P_ABORTAR) {
		log_info(logger, "Orden de abortar recibida en %d bytes", bytesRecibidos);
		finalizar(EXIT_FAILURE);
	} else {
		log_info(logger, "Se rompio el planificador! Arrivederci!! (o como se escriba)");
		finalizar(EXIT_FAILURE);
	}
}

tRespuestaCoordinador * recibirResultadoOperacion() {
	tRespuestaCoordinador *respuestaCoordinador = malloc(sizeof(tRespuestaCoordinador));
	tMensaje tipoMensajeCoordinador;
	char * sPayloadRespuestaCoordinador;

	log_info(logger, "Esperando respuesta de la operacion...");
	int recibidos = recibirPaquete(configuracion->socketCoordinador,&tipoMensajeCoordinador, &sPayloadRespuestaCoordinador, logger,"Respuesta a la ejecucion");
	log_info(logger, "RECIBIDOS:%d", recibidos);
	respuestaCoordinador->mensaje = malloc(100);

	deserializar(sPayloadRespuestaCoordinador, "%s",respuestaCoordinador->mensaje);
	free(sPayloadRespuestaCoordinador);

	log_info(logger, "RESPUESTA OPERACION DEL COORDINADOR : %s",respuestaCoordinador->mensaje);
	return respuestaCoordinador;
}

void cargarArchivo(char *path) {
	archivoScript = fopen(path, "r");
	if (archivoScript == NULL) {
		log_error(logger, "Error al abrir el archivo: %s", path);
		finalizar(EXIT_FAILURE);
	}
}

//este metodo se ejecuta cuando se recibe la orden de lectura del planificador
//evalua si hay que mandar la misma linea otra vez, o una nueva
//tambien evalua si el programa finalizo
void ordenRecibida() {
	if (lecturaRechazada) {
		enviarLineaOK(); //ENVIO OK AL PLANIFICADOR
		// todo usleep() ???
		usleep(20 * 1000);
		enviarOperacion(); //ENVIO AL COORDINADOR LA SENTENCIA
	} else {
		if (leerLinea() < 0) {
			enviarEsiFinalizado(); //ENVIO FIN DE LECTURA AL PLANIFICADOR
			finalizar(EXIT_SUCCESS);
		} else {
			enviarLineaOK(); //ENVIO OK AL PLANIFICADOR
			usleep(20 * 1000);
			enviarOperacion();//ENVIO AL COORDINADOR LA SENTENCIA
		}
	}
}

int leerLinea() {
	ssize_t lectura;
	if (operacion->clave != NULL) {
		free(operacion->clave);
		operacion->clave = NULL;
	}
	if (operacion->valor != NULL) {
		free(operacion->valor);
		operacion->valor = NULL;
	}
	/* ToDo me parece que antes de getline() hay que hacer esto
	  	linePtr = NULL;
	    lineLong = 0;
	 */
	if ((lectura = getline(&linePtr, &lineLong, archivoScript)) != -1) {

		t_esi_operacion lineaParseada = parse(linePtr);

		if (lineaParseada.valido) {
			switch (lineaParseada.keyword) {
			case GET:
				operacion->operacion = OPERACION_GET;
				operacion->clave = strdup(lineaParseada.argumentos.GET.clave);
				break;
			case SET:
				operacion->operacion = OPERACION_SET;
				operacion->clave = strdup(lineaParseada.argumentos.SET.clave);
				operacion->valor = strdup(lineaParseada.argumentos.SET.valor);
				break;
			case STORE:
				operacion->operacion = OPERACION_STORE;
				operacion->clave = strdup(lineaParseada.argumentos.STORE.clave);
				break;
			default:
				log_error(logger, "No pude interpretar <%s>\n", linePtr);
				finalizar(EXIT_FAILURE);

			}
			destruir_operacion(lineaParseada);

			log_info(logger, "\noperacion: %d \n clave: %s \n valor: %s\n",operacion->operacion, operacion->clave, operacion->valor);
		} else {
			log_error(logger, "La linea <%s> no es valida\n", linePtr);
			finalizar(EXIT_FAILURE);
		}
	} else {
		return -1;
	}
	return 0;
}

void enviarLineaOK() {
	tPaquete pkgLineaOk;
	int bytesEnviados;

	pkgLineaOk.type = E_LINEA_OK;

	pkgLineaOk.length = serializar(pkgLineaOk.payload, "%s", "OK");

	bytesEnviados = enviarPaquete(configuracion->socketPlanificador, &pkgLineaOk, logger, "Se envia OK al Planificador");
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

		pkgSentencia.length = serializar(pkgSentencia.payload, "%s",operacion->clave);

		log_info(logger, "Se envia la instruccion GET al coordinador");
		bytesEnviados = enviarPaquete(configuracion->socketCoordinador,&pkgSentencia, logger,"Se envia la instruccion GET al coordinador");
		log_info(logger, "Se envian %d bytes", bytesEnviados);

	} else if (operacion->operacion == OPERACION_SET) {
		pkgSentencia.type = E_SENTENCIA_SET;

		pkgSentencia.length = serializar(pkgSentencia.payload, "%s%s",operacion->clave, operacion->valor);

		log_info(logger, "Se envia la instruccion SET al coordinador");
		bytesEnviados = enviarPaquete(configuracion->socketCoordinador,&pkgSentencia, logger,"Se envia la instruccion SET al coordinador");
		log_info(logger, "Se envian %d bytes", bytesEnviados);

	} else if (operacion->operacion == OPERACION_STORE) {
		pkgSentencia.type = E_SENTENCIA_STORE;  //

		pkgSentencia.length = serializar(pkgSentencia.payload, "%s",operacion->clave);

		log_info(logger, "Se envia la instruccion STORE al coordinador");
		bytesEnviados = enviarPaquete(configuracion->socketCoordinador,&pkgSentencia, logger,"Se envia la instruccion STORE al coordinador");
		log_info(logger, "Se envian %d bytes", bytesEnviados);

	}

}

void enviarEsiFinalizado() {
	//en este metodo se informa al planificador que ya no hay lineas para leer
	//y el ESI finalizo su ejecucion
	tPaquete pkgSentencia;
	int bytesEnviados;

	pkgSentencia.type = E_ESI_FINALIZADO;
	pkgSentencia.length = serializar(pkgSentencia.payload, "%s", "FINALIZADO");
	log_info(logger, "Se envia respuesta Finalizado");
	bytesEnviados = enviarPaquete(configuracion->socketPlanificador, &pkgSentencia, logger, "Se envia respuesta Finalizado");
	log_info(logger, "Se envian %d bytes", bytesEnviados);
}
