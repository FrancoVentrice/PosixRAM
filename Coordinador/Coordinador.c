/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Coordinador.h"

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	inicializarSockets();
	levantarHiloEscucha();
	cicloPrincipal();
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}

void inicializarSockets() {
	socketEscucha = crearSocketEscucha(configuracion->puertoEscucha, logger);
	FD_ZERO(&setSockets);
	maxSock = 0;
}

void levantarHiloEscucha() {
	if (pthread_create(&hiloEscucha, NULL, escucharConexiones, NULL)) {
		log_error(logger, "Error al levantar el hilo de escucha de conexiones");
		finalizar(EXIT_FAILURE);
	}
}

void escucharConexiones() {
	while (1) {
		int iSocketComunicacion = getNewConnection(socketEscucha, &setSockets, &maxSock);
		//Nueva conexion aceptada
		if (iSocketComunicacion > 0) {
			//Se recibe el mensaje de handshake
			tMensaje tipoMensajeHandshake;
			char *buffer = malloc(50);
			int bytesRecibidos = recibirPaquete(iSocketComunicacion, &tipoMensajeHandshake, &buffer, logger, "Se recibe handshake");

			//Se procede segun el proceso que se conecto
			tPaquete pkgHandshake;
			pkgHandshake.type = C_HANDSHAKE;
			tRespuesta* respuestaHS = malloc(sizeof(tRespuesta));
			respuestaHS->mensaje = malloc(15);
			switch (tipoMensajeHandshake) {

			case P_HANDSHAKE:
				socketPlanificador = iSocketComunicacion;
				log_info(logger, "Handshake con Planificador, socket: %d", socketPlanificador);
				strcpy(respuestaHS->mensaje, "Handshake OK");
				pkgHandshake.length = serializar(pkgHandshake.payload, "%c%s", pkgHandshake.type, respuestaHS->mensaje);
				break;

			case E_HANDSHAKE:
				log_info(logger, "Handshake con ESI, socket: %d", iSocketComunicacion);
				strcpy(respuestaHS->mensaje, "Handshake OK");
				pkgHandshake.length = serializar(pkgHandshake.payload, "%c%s", pkgHandshake.type, respuestaHS->mensaje);
				break;

			case I_HANDSHAKE:
				log_info(logger, "a label can only be part of a statement and a declaration is not a statement. Si, esto sale a produccion");
				char *nombreInstancia = malloc(50);
				deserializar(buffer, "%s", nombreInstancia);
				log_info(logger, "Handshake con instancia %s, socket: %d", nombreInstancia, iSocketComunicacion);

				if (existeInstancia(nombreInstancia)) {
					pkgHandshake.length = serializar(pkgHandshake.payload, "%d;%d;%s", 0, 0, "Ya existe entrada conectada con el nombre indicado.");
				} else {
					pkgHandshake.length = serializar(pkgHandshake.payload, "%d;%d;%s", configuracion->cantidadDeEntradas, configuracion->tamanioDeEntrada, "Instancia aceptada");
					nuevaInstanciaConectada(nombreInstancia, iSocketComunicacion);
				}
				break;
			}

			//Se envia respuesta de handshake
			int bytesEnviados = enviarPaquete(iSocketComunicacion, &pkgHandshake, logger, "Se envia respuesta de handshake");
			log_info(logger, "Se envia respuesta de handshake de %d bytes\n", bytesEnviados);
			free(respuestaHS->mensaje);
			free(respuestaHS);
			free(buffer);
		}
	}
}

void cicloPrincipal() {
	operacion->clave=malloc(41);
	operacion->valor=malloc(100);
	char *respuestaConsulta = malloc(10);
	char *sPayloadRespuesta = malloc(100);

	while (1) {
		tMensaje tipoMensaje;
		fd_set temp;
		log_info(logger, "Escuchando");
		int iSocketComunicacion = multiplexarTimed(&setSockets, &temp, &maxSock, &tipoMensaje, &sPayloadRespuesta, logger, 1, 0);

		if (iSocketComunicacion != -1) {
			log_info(logger, "Mensaje recibido de socket %d, tipo %d", iSocketComunicacion, tipoMensaje);
			switch(tipoMensaje) {

			case E_SENTENCIA_GET:
				log_info(logger, "Se recibe sentencia GET a ser ejecutada");
				deserializar(sPayloadRespuesta, "%s", operacion->clave);
				operacion->operacion = OPERACION_GET;
				operacion->remitente = iSocketComunicacion;
				escribirLogDeOperaciones();
				consultarPlanificador();
				char *respuestaConsultaGet = recibirRespuestaConsulta(respuestaConsulta);
				accionarFrenteAConsulta(respuestaConsultaGet);
				tipoMensaje = DESCONEXION;
				break;

			case E_SENTENCIA_SET:
				log_info(logger, "Se recibe sentencia SET a ser ejecutada");
				log_info(logger, "Socket comunicacion: %d \n", iSocketComunicacion);
				deserializar(sPayloadRespuesta, "%s%s", operacion->clave, operacion->valor);
				operacion->operacion = OPERACION_SET;
				operacion->remitente = iSocketComunicacion;
				escribirLogDeOperaciones();
				consultarPlanificador();
				char *respuestaConsultaSet = recibirRespuestaConsulta(respuestaConsulta);
				accionarFrenteAConsulta(respuestaConsultaSet);
				tipoMensaje = DESCONEXION;
				break;

			case E_SENTENCIA_STORE:
				log_info(logger, "Se recibe sentencia STORE a ser ejecutada");
				log_info(logger, "Socket comunicacion: %d \n", iSocketComunicacion);
				deserializar(sPayloadRespuesta, "%s", operacion->clave);
				operacion->operacion = OPERACION_STORE;
				operacion->remitente = iSocketComunicacion;
				escribirLogDeOperaciones();
				consultarPlanificador();
				char *respuestaConsultaStore = recibirRespuestaConsulta(respuestaConsulta);
				accionarFrenteAConsulta(respuestaConsultaStore);
				tipoMensaje = DESCONEXION;
				break;

			case P_RESPUESTA_CONSULTA:
				log_info(logger,"Respuesta Consulta Operacion");
				char *respuesta = malloc(10);
				recibirRespuestaConsulta(respuesta);
				tipoMensaje = DESCONEXION;
				break;

			case P_ESTADO_CLAVE:
				log_info(logger,"Consulta estado de clave");
				char *claveConsultada = malloc(41);
				deserializar(sPayloadRespuesta, "%s", claveConsultada);
				evaluarEstadoDeClave(claveConsultada);
				break;

			case DESCONEXION:
				break;

			}
		}
	}
	finalizar(0);
}

void consultarPlanificador() {
	tPaquete pkgConsulta;
	int enviados;
	switch(operacion->operacion){

	case OPERACION_GET:
		pkgConsulta.type = C_CONSULTA_OPERACION_GET;
		pkgConsulta.length = serializar(pkgConsulta.payload, "%s",
				operacion->clave);

		log_info(logger,"Se consulta al planificador GET");
		enviados = enviarPaquete(socketPlanificador, &pkgConsulta,
				logger, "Se consulta al planificador");
		log_info(logger,"Se envian %d bytes\n", enviados);
		break;

	case OPERACION_SET:
		pkgConsulta.type = C_CONSULTA_OPERACION_SET;

		pkgConsulta.length = serializar(pkgConsulta.payload, "%s", operacion->clave);

		log_info(logger, "Se consulta al planificador SET");
		enviados = enviarPaquete(socketPlanificador, &pkgConsulta, logger,
				"Se consulta al planificador");
		log_info(logger, "Se envian %d bytes\n", enviados);
		break;

	case OPERACION_STORE:
		pkgConsulta.type = C_CONSULTA_OPERACION_STORE;

		pkgConsulta.length = serializar(pkgConsulta.payload, "%s",
				operacion->clave);

		log_info(logger, "Se consulta al planificador STORE");
		enviados = enviarPaquete(socketPlanificador, &pkgConsulta, logger,
				"Se consulta al planificador");
		log_info(logger, "Se envian %d bytes\n", enviados);
		break;
	}


}

char * recibirRespuestaConsulta(char *respuesta) {
	tMensaje tipoMensajeEsi;
	char *respuestaConsulta = malloc(100);

	int bytesRecibidos = recibirPaquete(socketPlanificador,
			&tipoMensajeEsi, &respuestaConsulta, logger, "Respuesta Consulta");
	log_info(logger, "RECIBIDOS:%d", bytesRecibidos);

	deserializar(respuestaConsulta, "%s", respuesta);
	log_info(logger, "Respuesta Consulta Planificador: %s", respuesta);
	return respuesta;
}

void accionarFrenteAConsulta(char * respuesta) {
	if (strcmp(respuesta, "OK") == 0) {
		if (operacion->operacion != OPERACION_GET) {
			log_info(logger, "eligiendo instancia");
			instanciaElegida = elegirInstancia(false);
			log_info(logger, "Instancia elegida: %s", instanciaElegida->nombre);
			enviarOperacionAInstancia();
			recibirOperacionDeInstancia();
		} else {
			registrarClaveAgregadaAInstancia();
			informarResultadoOperacionOk(operacion->remitente);
		}
	} else if (strcmp(respuesta, "BLOQUEADO") == 0) {
		informarResultadoOperacionBloqueado(operacion->remitente);
	} else if (strcmp(respuesta, "ERROR") == 0) {
		informarResultadoOperacionError(operacion->remitente);
	}
	free(respuesta);
}

void enviarOperacionAInstancia() {
	//aca se envia la operacion a ejecutar a la instancia elegida
	//datos utiles:
	//
	//instanciaElegida->socket para enviar el mensaje
	//
	//operacion->operacion, operacion->clave y operacion->valor

	tPaquete pkgOperacion;

	int bytesEnviados;
	pkgOperacion.type = I_RESULTADO_SET;

	pkgOperacion.length = serializar(pkgOperacion.payload, "%s%s", operacion->clave,operacion->valor);

	bytesEnviados = enviarPaquete(instanciaElegida->socket, &pkgOperacion,
			logger, "Se envia OK al Planificador");
	log_info(logger, "Se envian %d bytes", bytesEnviados);

}

void recibirOperacionDeInstancia() {
	//aca se recibe el resultado de la operacion
	//
	//
	//tiene que contener tambien la cantidad de entradas disponibles que le quedan a la instancia
	//instanciaElegida->cantidadDeEntradasDisponibles = algo como "respuesta->entradasDisponibles"

	//if (OK) informarResultadoOperacionOk();
	char* resultadoOkSet=malloc(5);
	char* respuesta=malloc(5);
	tMensaje tipoMensajeEsi;


	int bytesRecibidos = recibirPaquete(socketPlanificador,
			&tipoMensajeEsi, &resultadoOkSet, logger, "Respuesta Consulta");
	log_info(logger, "RECIBIDOS:%d", bytesRecibidos);

	deserializar(resultadoOkSet, "%s", respuesta);
	log_info(logger, "Respuesta Consulta Planificador: %s", respuesta);

}

void escribirLogDeOperaciones() {
	char *stringOperacion = malloc(100);
	switch (operacion->operacion) {
	case OPERACION_GET:
		strcpy(stringOperacion, string_from_format("GET %s", operacion->clave));
		break;
	case OPERACION_SET:
		strcpy(stringOperacion, string_from_format("SET %s %s", operacion->clave, operacion->valor));
		break;
	case OPERACION_STORE:
		strcpy(stringOperacion, string_from_format("STORE %s", operacion->clave));
		break;
	}
	log_info(logDeOperaciones, "Proceso de operacion: %s", stringOperacion);
	free(stringOperacion);
}

void informarResultadoOperacionOk(int socketEsi){
	//ENVIO RESPUESTA AL ESI
	int bytesEnviados;
	tSolicitudESI* resultadoOperacion = malloc(sizeof(tSolicitudESI));
	resultadoOperacion->mensaje = malloc(100);
	strcpy(resultadoOperacion->mensaje, "OK");
	tPaquete pkgHandshake2;
	pkgHandshake2.type = C_RESULTADO_OPERACION;

	pkgHandshake2.length = serializar(pkgHandshake2.payload, "%s", resultadoOperacion->mensaje);

	log_info(logger,"Se envia respuesta al ESI");
	bytesEnviados = enviarPaquete(socketEsi, &pkgHandshake2, logger,
			"Se envia respuesta al ESI");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

	//ENVIO RESPUESTA AL PLANIFICADOR

	log_info(logger,"Se envia respuesta al Planificador");
	bytesEnviados = enviarPaquete(socketPlanificador, &pkgHandshake2, logger,
			"Se envia respuesta al Planificador");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

}

void informarResultadoOperacionBloqueado(int socketEsi){
	//ENVIO RESPUESTA AL ESI
	int bytesEnviados;
	tSolicitudESI* resultadoOperacion = malloc(sizeof(tSolicitudESI));
	resultadoOperacion->mensaje = malloc(100);
	strcpy(resultadoOperacion->mensaje, "BLOQUEADO");
	tPaquete pkgHandshake2;
	pkgHandshake2.type = C_RESULTADO_OPERACION;

	pkgHandshake2.length = serializar(pkgHandshake2.payload, "%s", resultadoOperacion->mensaje);

	log_info(logger,"Se envia respuesta al ESI");
	bytesEnviados = enviarPaquete(socketEsi, &pkgHandshake2, logger,
			"Se envia respuesta al ESI");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

	//ENVIO RESPUESTA AL PLANIFICADOR

	log_info(logger,"Se envia respuesta al Planificador");
	bytesEnviados = enviarPaquete(socketPlanificador, &pkgHandshake2, logger,
			"Se envia respuesta al Planificador");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

}

void informarResultadoOperacionError(int socketEsi){
	//ENVIO RESPUESTA AL ESI
	int bytesEnviados;
	tSolicitudESI * resultadoOperacion = malloc(sizeof(tSolicitudESI));
	resultadoOperacion->mensaje = malloc(100);
	strcpy(resultadoOperacion->mensaje, "ERROR");
	tPaquete pkgHandshake2;
	pkgHandshake2.type = C_RESULTADO_OPERACION;

	pkgHandshake2.length = serializar(pkgHandshake2.payload, "%s", resultadoOperacion->mensaje);

	log_info(logger,"Se envia respuesta al ESI");
	bytesEnviados = enviarPaquete(socketEsi, &pkgHandshake2, logger,
			"Se envia respuesta al ESI");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

	//ENVIO RESPUESTA AL PLANIFICADOR

	log_info(logger,"Se envia respuesta al Planificador");
	bytesEnviados = enviarPaquete(socketPlanificador, &pkgHandshake2, logger,
			"Se envia respuesta al Planificador");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

}

//se usa para elegir la instancia en la cual ejecutar la operacion
//elige la instancia a la cual le va a agregar o modificar la clave
t_instancia * elegirInstancia(bool consulta) {
	log_info(logger, "eligiendo instancia para clave %s", operacion->clave);
	if (!dictionary_has_key(diccionarioClaves, operacion->clave)) {
		log_info(logger, "algoritmoDistribucion %d", configuracion->algoritmoDistribucion);
		switch (configuracion->algoritmoDistribucion) {
		case ALGORITMO_LSU:
			return elegirInstanciaLSU();
			break;
		case ALGORITMO_EL:
			return elegirInstanciaEL(consulta);
			break;
		case ALGORITMO_KE:
			return elegirInstanciaKE();
			break;
		}
	} else {
		log_info(logger, "dictionary has key!");
		return dictionary_get(diccionarioClaves, operacion->clave);
	}
	return NULL;
}

//metodo que se ejecuta para registrar una nueva clave a una instancia
//se ejecuta en el GET
void registrarClaveAgregadaAInstancia() {
	if (!dictionary_has_key(diccionarioClaves, operacion->clave)) {
		dictionary_put(diccionarioClaves, operacion->clave, elegirInstancia(false));
	}
}

//LEAST SPACE USED
//usa la clave de operacion (variable global)
//devuelve la instancia elegida (variable global)
//
//
//To Do: la respuesta de una instancia a un SET deberia devolver la cantidad
//de entradas disponibles asi el valor se mantiene actualizado para este algoritmo
//
//
t_instancia * elegirInstanciaLSU() {
	int i;
	t_instancia *elegida = list_get(instancias, 0);
	for (i = 1; i < instancias->elements_count; i++) {
		t_instancia *iinstancia = list_get(instancias, i);
		if (iinstancia->cantidadDeEntradasDisponibles > elegida->cantidadDeEntradasDisponibles) {
			elegida = iinstancia;
		}
	}
	log_info(logger, "instancia elegida (LSU): %s", elegida->nombre);
	return elegida;
}

//EQUITATIVE LOAD
//usa la clave de operacion (variable global)
//devuelve la instancia elegida (variable global)
t_instancia * elegirInstanciaEL(bool consulta) {
	int punteroConsulta;
	if (consulta) {
		punteroConsulta = punteroEL;
	}
	if (punteroEL >= instancias->elements_count) {
		punteroEL = 0;
	}
	t_instancia *elegida = list_get(instancias, punteroEL);
	punteroEL ++;
	log_info(logger, "instancia elegida (EL): %s", elegida->nombre);

	if (consulta) {
		punteroEL = punteroConsulta;
	}
	return elegida;
}

//KEY EXPLICIT
//usa la clave de operacion (variable global)
//devuelve la instancia elegida (variable global)
//valor de 'a' es 97
//valor de 'z' es 122
//valor de 'A' es 65
//valor de 'Z' es 90
//122 - 97 = 25 letras a ser distribuidas entre las instancias
t_instancia * elegirInstanciaKE() {
	if (instancias->elements_count == 0) {
		log_info(logger, "no hay instancias para elegir");
		return NULL;
	}
	int letrasPorInstancia = 26 / instancias->elements_count;
	if (26 % instancias->elements_count > 0) {
		letrasPorInstancia++;
	}
	//'a' tendria valor 1, 'b' 2, y asi
	int valorPrimerCaracter = operacion->clave[0];
	if (valorPrimerCaracter < 91) {
		valorPrimerCaracter -= 64;
	} else {
		valorPrimerCaracter -= 96;
	}
	int indexInstancia = valorPrimerCaracter / letrasPorInstancia;
	if (valorPrimerCaracter % letrasPorInstancia == 0) {
		indexInstancia--;
	}
	t_instancia *elegida = list_get(instancias, indexInstancia);
	log_info(logger, "instancia elegida (KE): %s", elegida->nombre);
	return elegida;
}

void instanciaDestroyer(t_instancia * instancia) {
	free(instancia->nombre);
	free(instancia);
}


//cuando entra una nueva instancia la agrega a la lista
void nuevaInstanciaConectada(char * nombreInstancia, int socketInst) {
	t_instancia *instancia = malloc(sizeof(t_instancia));

	instancia->nombre = nombreInstancia;
	instancia->cantidadDeEntradasDisponibles = configuracion->cantidadDeEntradas;
	instancia->socket = socketInst;
	list_add(instancias, instancia);
}

int existeInstancia(char *nombreInstancia) {
	/* revisar uso de list_any_satisfy() */
	int i;
	for (i = 0; i < list_size(instancias); i++) {
		t_instancia *instancia = list_get(instancias, i);
		if (string_equals_ignore_case(instancia->nombre, nombreInstancia)) {
			return 1;
		}
	}
	return 0;
}

void evaluarEstadoDeClave(char *claveConsultada) {
	t_instancia *instancia = elegirInstancia(true);

	//Respondo lo averiguado
	//ToDo: hacer consulta con la instancia para saber el valor de la clave
	char *valor = strdup("VALOR MOCKEADO");
	char *nombreInstancia = instancia->nombre;
	tPaquete pkgEstadoClave;
	pkgEstadoClave.type = C_ESTADO_CLAVE;
	pkgEstadoClave.length = serializar(pkgEstadoClave.payload, "%s%s", valor, nombreInstancia);
	log_info(logger,"Se envia estado de clave %s, valor %s, instancia %s", claveConsultada, valor, nombreInstancia);
	int bytesEnviados = enviarPaquete(socketPlanificador, &pkgEstadoClave, logger, "Se envia estado de clave");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);
}
