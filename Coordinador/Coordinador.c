/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Coordinador.h"

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	escucharConexiones();
}
void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}

void escucharConexiones() {

	int puertoEscucha = configuracion->puertoEscucha;
	int maxSock;
	int iSocketEscucha;
	int iSocketComunicacion;
	int tamanioTotal = 0;
	operacion->clave=malloc(100);
	operacion->valor=malloc(100);
	char *respuestaConsulta = malloc(10);



	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log
	iSocketEscucha = crearSocketEscucha(puertoEscucha, logger);

	FD_SET(iSocketEscucha, &setSocketsOrquestador);
	maxSock = iSocketEscucha;

	tMensaje *tipoMensaje = malloc(sizeof(tMensaje));
	char * sPayloadRespuesta = malloc(100);

	tSolicitudESI *solicitud = malloc(sizeof(tSolicitudESI));
	solicitud->mensaje = malloc(100);

	puts("Escuchando");

	while (1) {
		iSocketComunicacion = getConnection(&setSocketsOrquestador, &maxSock,
				iSocketEscucha, tipoMensaje, &sPayloadRespuesta,
				logger);

		if (iSocketComunicacion != -1) {

			switch (*tipoMensaje) {

			case E_HANDSHAKE:
				log_info(logger,"Socket comunicacion: %d \n", iSocketComunicacion);
				log_info(logger,"HANDSHAKE CON ESI");
				char* encabezado = malloc(10);
				char encabezadoMensaje;


				deserializar(sPayloadRespuesta, "%c%s",&encabezadoMensaje, solicitud->mensaje);
				log_info(logger,"MENSAJE DE ESI: %s\n", solicitud->mensaje);


				//RESPUESTA HANDSHAKE
				tSolicitudESI* solicitudESI = malloc(sizeof(tSolicitudESI));
				solicitudESI->mensaje = malloc(100);
				strcpy(solicitudESI->mensaje, "HANDSHAKE OK");
				tPaquete pkgHandshakeESI;
				pkgHandshakeESI.type = C_HANDSHAKE;

				pkgHandshakeESI.length = serializar(pkgHandshakeESI.payload, "%c%s",
						pkgHandshakeESI.type, solicitudESI->mensaje);

				puts("Se envia respuesta al ESI");
				tamanioTotal = enviarPaquete(iSocketComunicacion,
						&pkgHandshakeESI, logger,
						"Se envia solicitud de ejecucion");
				printf("Se envian %d bytes\n", tamanioTotal);

				*tipoMensaje = DESCONEXION;
				break;


			case E_SENTENCIA_GET:

				log_info(logger, "Se recibe sentencia GET a ser ejecutada");

				log_info(logger, "Socket comunicacion: %d \n",
						iSocketComunicacion);

				deserializar(sPayloadRespuesta, "%s", operacion->clave);
				operacion->operacion = OPERACION_GET;
				operacion->remitente = iSocketComunicacion;
				escribirLogDeOperaciones();

				consultarPlanificador();
				char *respuestaConsultaGet = recibirRespuestaConsulta(respuestaConsulta);
				accionarFrenteAConsulta(respuestaConsultaGet);
				*tipoMensaje = DESCONEXION;
				break;

			case E_SENTENCIA_SET:

				log_info(logger, "Se recibe sentencia SET a ser ejecutada");

					log_info(logger, "Socket comunicacion: %d \n",
						iSocketComunicacion);

				deserializar(sPayloadRespuesta, "%s%s", operacion->clave,
						operacion->valor);
				operacion->operacion = OPERACION_SET;
				operacion->remitente = iSocketComunicacion;
				escribirLogDeOperaciones();

				consultarPlanificador();
				char *respuestaConsultaSet = recibirRespuestaConsulta(respuestaConsulta);
				accionarFrenteAConsulta(respuestaConsultaSet);

				*tipoMensaje = DESCONEXION;
				break;

			case E_SENTENCIA_STORE:

				log_info(logger, "Se recibe sentencia SET a ser ejecutada");

				log_info(logger, "Socket comunicacion: %d \n",
						iSocketComunicacion);

				deserializar(sPayloadRespuesta, "%s", operacion->clave);
				operacion->operacion = OPERACION_STORE;
				operacion->remitente = iSocketComunicacion;
				escribirLogDeOperaciones();

				consultarPlanificador();
				char *respuestaConsultaStore = recibirRespuestaConsulta(respuestaConsulta);
				accionarFrenteAConsulta(respuestaConsultaStore);

				*tipoMensaje = DESCONEXION;

				break;
			case P_HANDSHAKE:
				printf("Socket comunicacion: %d \n", iSocketComunicacion);
				socketPlanificador = iSocketComunicacion;
				puts("HANDSHAKE CON PLANIFICADOR");
				tSolicitudESI* solicitud = malloc(sizeof(tSolicitudESI));
				solicitud->mensaje = malloc(100);
				strcpy(solicitud->mensaje, "OK HANDSHAKE");
				tPaquete pkgHandshake2;
				pkgHandshake2.type = C_HANDSHAKE;

				pkgHandshake2.length = serializar(pkgHandshake2.payload, "%c%s",
						pkgHandshake2.type, solicitud->mensaje);

				puts("Se envia respuesta al Planificador");
				tamanioTotal = enviarPaquete(iSocketComunicacion,
						&pkgHandshake2, logger,
						"Se envia solicitud de ejecucion");
				printf("Se envian %d bytes\n", tamanioTotal);

				//Recibo respuesta sobre la consulta



				*tipoMensaje = DESCONEXION;
				break;


			case C_HANDSHAKE:
				/* se agrega para que no genere warning
				 * el coordinador no se saluda a sí mismo
				 */
				break;

			case I_HANDSHAKE:
				// TODO revisar lo que se imprime en pantalla y mandarlo al log
				puts("Se conectó una Instancia. Recibiendo handshake.");
				char * nombreInstancia = (char *)malloc(50);
				deserializar(sPayloadRespuesta, "%s", nombreInstancia);
				printf("Instancia: %s\n", nombreInstancia);

				puts("Respondiendo handshake de Instancia");
				tPaquete pkgHandshakeResponse;
				pkgHandshakeResponse.type = C_HANDSHAKE;
				if (existeInstancia(nombreInstancia))
					pkgHandshakeResponse.length = serializar(pkgHandshakeResponse.payload, "%d;%d;%s",0,0,"Ya existe entrada conectada con el nombre indicado.");
				else
					pkgHandshakeResponse.length = serializar(pkgHandshakeResponse.payload, "%d;%d;%s",configuracion->cantidadDeEntradas,configuracion->tamanioDeEntrada,"Instancia aceptada");
				tamanioTotal = enviarPaquete(iSocketComunicacion,&pkgHandshakeResponse, logger,"Respondiendo handshake de Instancia");
				printf("Se envian %d bytes\n", tamanioTotal);

				nuevaInstanciaConectada(nombreInstancia, iSocketComunicacion);

				// TODO liberar nombreInstancia cuando se termina de crear la instancia

				*tipoMensaje = DESCONEXION;
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

		pkgConsulta.length = serializar(pkgConsulta.payload, "%s%s",
						operacion->clave,operacion->valor);

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
			instanciaElegida = elegirInstancia();
			enviarOperacionAInstancia();
			recibirOperacionDeInstancia();
		} else {
			registrarClaveAgregadaAInstancia();
			informarResultadoOperacionOk(operacion->remitente);
		}
	} else {
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
}

void recibirOperacionDeInstancia() {
	//aca se recibe el resultado de la operacion
	//
	//
	//tiene que contener tambien la cantidad de entradas disponibles que le quedan a la instancia
	//instanciaElegida->cantidadDeEntradasDisponibles = algo como "respuesta->entradasDisponibles"

	//if (OK) informarResultadoOperacionOk();
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
t_instancia * elegirInstancia() {
	if (!dictionary_has_key(diccionarioClaves, operacion->clave)) {
		switch (configuracion->algoritmoDistribucion) {
		case ALGORITMO_LSU:
			return elegirInstanciaLSU();
			break;
		case ALGORITMO_EL:
			return elegirInstanciaEL();
			break;
		case ALGORITMO_KE:
			return elegirInstanciaKE();
			break;
		}
	} else {
		return dictionary_get(diccionarioClaves, operacion->clave);
	}
}

//metodo que se ejecuta para registrar una nueva clave a una instancia
//se ejecuta en el GET
void registrarClaveAgregadaAInstancia() {
	if (!dictionary_has_key(diccionarioClaves, operacion->clave)) {
		dictionary_put(diccionarioClaves, operacion->clave, elegirInstancia());
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
t_instancia * elegirInstanciaEL() {
	if (punteroEL >= instancias->elements_count) {
		punteroEL = 0;
	}
	t_instancia *elegida = list_get(instancias, punteroEL);
	punteroEL ++;
	log_info(logger, "instancia elegida (EL): %s", elegida->nombre);
	return elegida;
}

//KEY EXPLICIT
//usa la clave de operacion (variable global)
//devuelve la instancia elegida (variable global)
//valor de 'a' es 97
//valor de 'z' es 122
//122 - 97 = 25 letras a ser distribuidas entre las instancias
t_instancia * elegirInstanciaKE() {
	if (instancias->elements_count == 0) {
		return NULL;
	}
	int letrasPorInstancia = 25 / instancias->elements_count;
	if (25 % instancias->elements_count > 0) {
		letrasPorInstancia++;
	}
	//'a' tendria valor 1, 'b' 2, y asi
	int valorPrimerCaracter = operacion->clave[0] - 96;
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
