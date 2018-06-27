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
			switch (tipoMensajeHandshake) {

			case P_HANDSHAKE:
				socketPlanificador = iSocketComunicacion;
				log_info(logger, "Handshake con Planificador, socket: %d", socketPlanificador);
				pkgHandshake.length = serializar(pkgHandshake.payload, "", NULL);
				break;

			case E_HANDSHAKE:
				log_info(logger, "Handshake con ESI, socket: %d", iSocketComunicacion);
				pkgHandshake.length = serializar(pkgHandshake.payload, "", NULL);
				break;

			case I_HANDSHAKE:
				log_info(logger, "a label can only be part of a statement and a declaration is not a statement. Si, esto sale a produccion");
				char *nombreInstancia = malloc(50);
				deserializar(buffer, "%s", nombreInstancia);
				log_info(logger, "Handshake con instancia %s, socket: %d", nombreInstancia, iSocketComunicacion);

				if (existeInstanciaConectadaConMismoNombre(nombreInstancia)) {
					// ya existe una instancia conectada con el mismo nombre --> se rechaza
					pkgHandshake.length = serializar(pkgHandshake.payload, "%d%d%s%s", 0, 0, "Ya existe instancia conectada con el nombre indicado.", "");
				}
				else {
					// instancia aceptada
					if (existeInstanciaDesconectadaConMismoNombre(nombreInstancia)) {
						// es la misma instancia que se está reconectndo --> se acepta y se envía la lista de claves
						char *clavesSincro = buscarClavesPorInstancia(nombreInstancia);
						pkgHandshake.length = serializar(pkgHandshake.payload, "%d%d%s%s", configuracion->cantidadDeEntradas, configuracion->tamanioDeEntrada,
								"Instancia reconectada. Bienvenida nuevamente!", clavesSincro);
						instanciaReconectada(nombreInstancia, iSocketComunicacion);
					}
					else {
						// instancia nueva
						pkgHandshake.length = serializar(pkgHandshake.payload, "%d%d%s%s", configuracion->cantidadDeEntradas, configuracion->tamanioDeEntrada, "Instancia nueva aceptada. Bienvenida!", "");
						nuevaInstanciaConectada(nombreInstancia, iSocketComunicacion);
					}
				}
				break;
			default:
				// por acá entran todos los tipos de mensajes que nos están faltando, para que no tire warnings
				break;
			}

			//Se envia respuesta de handshake
			int bytesEnviados = enviarPaquete(iSocketComunicacion, &pkgHandshake, logger, "Se envia respuesta de handshake");
			log_info(logger, "Se envia respuesta de handshake de %d bytes\n", bytesEnviados);
			free(buffer);
			// TODO ¿dónde se guarda el nuevo socket del proceso que se conectó?
		}
	}
}

void cicloPrincipal() {
	operacion->clave=malloc(MAX_LONG_CLAVE);
	operacion->valor=malloc(100);
	char *respuestaConsulta = malloc(10);
	char *sPayloadRespuesta = malloc(100);

	while (1) {
		tMensaje tipoMensaje;
		fd_set temp;
		//log_info(logger, "Escuchando");
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
				char *claveConsultada = malloc(MAX_LONG_CLAVE);
				deserializar(sPayloadRespuesta, "%s", claveConsultada);
				evaluarEstadoDeClave(claveConsultada);
				break;

			case DESCONEXION:
				validarDesconexionDeInstancia(iSocketComunicacion);
				break;
			default:
				// por acá entran todos los mensajes que nos estén faltando, para que no tire warnings
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
			informarResultadoOperacionOk();
		}
	} else if (strcmp(respuesta, "BLOQUEADO") == 0) {
		informarResultadoOperacionBloqueado();
	} else if (strcmp(respuesta, "ERROR") == 0) {
		informarResultadoOperacionError();
	}
	free(respuesta);
}

void enviarOperacionAInstancia() {
	tPaquete pkgOperacion;
	int bytesEnviados;
	if (operacion->operacion == OPERACION_SET) {
		pkgOperacion.type = C_EJECUTAR_SET;
		pkgOperacion.length = serializar(pkgOperacion.payload, "%s%s", operacion->clave,operacion->valor);
	} else if (operacion->operacion == OPERACION_STORE) {
		pkgOperacion.type = C_EJECUTAR_STORE;
		pkgOperacion.length = serializar(pkgOperacion.payload, "%s", operacion->clave);
	}

	bytesEnviados = enviarPaquete(instanciaElegida->socket, &pkgOperacion, logger, "Se envia operacion a instancia");
	log_info(logger, "Se envian %d bytes a instancia con socket %d", bytesEnviados, instanciaElegida->socket);
}

void recibirOperacionDeInstancia() {
	char* resultadoInstancia = malloc(60);
	tMensaje tipoMensaje;
	log_info(logger, "esperando respuesta de socket %d", instanciaElegida->socket);
	int bytesRecibidos = recibirPaquete(instanciaElegida->socket, &tipoMensaje, &resultadoInstancia, logger, "Respuesta instancia");
	log_info(logger, "RECIBIDOS:%d", bytesRecibidos);

	if (tipoMensaje == I_RESULTADO_SET) {
		int entradasLibres = 0;
		char* clave = malloc(41);
		char charCompactacion = 0;
		deserializar(resultadoInstancia, "%d%s%c", &entradasLibres, clave, &charCompactacion);
		instanciaElegida->cantidadDeEntradasDisponibles = entradasLibres;

		log_info(logger, "\nentradas libres: %d \nclave: %s \ncompactacion: %d", entradasLibres, clave, charCompactacion);

		//Si la clave recibida es distinta a la clave de la operacion, es porque fue reemplazada.
		//Por lo tanto, elimino la clave reemplazada del diccionario
		if (strcmp(clave, operacion->clave) != 0 && dictionary_has_key(diccionarioClaves, clave)) {
			dictionary_remove(diccionarioClaves, clave);
		}

		//Si la instancia tuvo que compactar para ejecutar la operacion,
		//aviso y ordeno a las demas instancias lo mismo
		if (charCompactacion) {
			ejecutarCompactacion();
		}

		informarResultadoOperacionOk(operacion->remitente);
	} else if (tipoMensaje == I_RESULTADO_STORE) {
		informarResultadoOperacionOk(operacion->remitente);
	} else if (tipoMensaje == DESCONEXION) {
		validarDesconexionDeInstancia(instanciaElegida->socket);
		informarResultadoOperacionError(operacion->remitente);
	} else if (tipoMensaje == I_RESULTADO_ERROR) {
		informarResultadoOperacionError(operacion->remitente);
	}
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

void informarResultadoOperacionOk() {
	//aplico retardo!
	retardoMilisegundos(configuracion->retardo);

	//preparo paquete
	int bytesEnviados;
	tSolicitudESI* resultadoOperacion = malloc(sizeof(tSolicitudESI));
	resultadoOperacion->mensaje = malloc(100);
	strcpy(resultadoOperacion->mensaje, "OK");
	tPaquete pkgHandshake2;
	pkgHandshake2.type = C_RESULTADO_OPERACION;
	pkgHandshake2.length = serializar(pkgHandshake2.payload, "%s", resultadoOperacion->mensaje);

	//envio respuesta al ESI
	log_info(logger,"Se envia respuesta al ESI");
	bytesEnviados = enviarPaquete(operacion->remitente, &pkgHandshake2, logger, "Se envia respuesta al ESI");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

	//envio respuesta al planificador
	log_info(logger,"Se envia respuesta al Planificador");
	bytesEnviados = enviarPaquete(socketPlanificador, &pkgHandshake2, logger, "Se envia respuesta al Planificador");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);
}

void informarResultadoOperacionBloqueado() {
	//aplico retardo!
	retardoMilisegundos(configuracion->retardo);

	//preparo paquete
	int bytesEnviados;
	tSolicitudESI* resultadoOperacion = malloc(sizeof(tSolicitudESI));
	resultadoOperacion->mensaje = malloc(100);
	strcpy(resultadoOperacion->mensaje, "BLOQUEADO");
	tPaquete pkgHandshake2;
	pkgHandshake2.type = C_RESULTADO_OPERACION;
	pkgHandshake2.length = serializar(pkgHandshake2.payload, "%s", resultadoOperacion->mensaje);

	//envio respuesta al ESI
	log_info(logger,"Se envia respuesta al ESI");
	bytesEnviados = enviarPaquete(operacion->remitente, &pkgHandshake2, logger, "Se envia respuesta al ESI");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

	//envio respuesta al planificador
	log_info(logger,"Se envia respuesta al Planificador");
	bytesEnviados = enviarPaquete(socketPlanificador, &pkgHandshake2, logger, "Se envia respuesta al Planificador");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);
}

void informarResultadoOperacionError() {
	//aplico retardo!
	retardoMilisegundos(configuracion->retardo);

	//preparo paquete
	int bytesEnviados;
	tSolicitudESI * resultadoOperacion = malloc(sizeof(tSolicitudESI));
	resultadoOperacion->mensaje = malloc(100);
	strcpy(resultadoOperacion->mensaje, "ERROR");
	tPaquete pkgHandshake2;
	pkgHandshake2.type = C_RESULTADO_OPERACION;
	pkgHandshake2.length = serializar(pkgHandshake2.payload, "%s", resultadoOperacion->mensaje);

	//envio respuesta al ESI
	log_info(logger,"Se envia respuesta al ESI");
	bytesEnviados = enviarPaquete(operacion->remitente, &pkgHandshake2, logger, "Se envia respuesta al ESI");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);

	//envio respuesta al planificador
	log_info(logger,"Se envia respuesta al Planificador");
	bytesEnviados = enviarPaquete(socketPlanificador, &pkgHandshake2, logger, "Se envia respuesta al Planificador");
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
	instancia->socket = socketInst;
	instancia->cantidadDeEntradasDisponibles = configuracion->cantidadDeEntradas;
	list_add(instancias, instancia);
}

int existeInstanciaConectadaConMismoNombre(char *nombreInstancia) {
	/* verifica si ya existe una instancia conectada con el mismo nombre */
	// TODO revisar uso de list_any_satisfy()
	int i;
	for (i = 0; i < list_size(instancias); i++) {
		t_instancia *instancia = list_get(instancias, i);
		if (string_equals_ignore_case(instancia->nombre, nombreInstancia) && (instancia->socket > 0)) {
			return 1;
		}
	}
	return 0;
}

void instanciaReconectada(char *nombreInstancia, int socketInst) {
	/* verifica si existe una instancia desconectada con el mismo nombre */

	int i;
	for (i = 0; i < list_size(instancias); i++) {
		t_instancia *instancia = list_get(instancias, i);
		if (string_equals_ignore_case(instancia->nombre, nombreInstancia)) {
			instancia->socket = socketInst;
		}
	}

}

int existeInstanciaDesconectadaConMismoNombre(char *nombreInstancia) {
	/* verifica si existe una instancia desconectada con el mismo nombre */

	int i;
	for (i = 0; i < list_size(instancias); i++) {
		t_instancia *instancia = list_get(instancias, i);
		if (string_equals_ignore_case(instancia->nombre, nombreInstancia) && (instancia->socket < 0)) {
			return 1;
		}
	}
	return 0;
}

int validarDesconexionDeInstancia(int socketInst) {
	/* chequea si la desconexión fue de una instancia, y en ese caso actualiza su estado */
	int i;
	log_info(logger, "Verificando si se desconectó alguna Instancia....");
	for (i = 0; i < list_size(instancias); i++) {
		t_instancia *instancia = list_get(instancias, i);
		if (instancia->socket == socketInst) {
			log_info(logger, "Instancia %s desconectada (%d entradas disponibles).", instancia->nombre, instancia->cantidadDeEntradasDisponibles);
			instancia->socket = -1; // le desasigno el socket pero la dejo en la lista, para permitir reconexión
			return 1;
		}
	}
	return 0;
}

void evaluarEstadoDeClave(char *claveConsultada) {
	t_instancia *instancia = elegirInstancia(true);
	char *valor;

	//Consulto con instancia el valor de la clave
	//(en caso de existir)
	if (dictionary_has_key(diccionarioClaves, claveConsultada)) {
		//preparo paquete
		tPaquete pkgEstadoClaveInstancia;
		pkgEstadoClaveInstancia.type = C_ESTADO_CLAVE;
		pkgEstadoClaveInstancia.length = serializar(pkgEstadoClaveInstancia.payload, "%s", claveConsultada);

		//envio consulta a Instancia
		log_info(logger,"Se envia consulta a Instancia");
		int bytesEnviados = enviarPaquete(instancia->socket, &pkgEstadoClaveInstancia, logger, "Se envia consulta a Instancia");
		log_info(logger,"Se envian %d bytes\n", bytesEnviados);

		//recibo respuesta de Instancia
		tMensaje tipoMensajeConsulta;
		char *respuestaConsulta = malloc(300);
		int bytesRecibidos = recibirPaquete(instancia->socket, &tipoMensajeConsulta, &respuestaConsulta, logger, "Respuesta consulta");
		log_info(logger, "RECIBIDOS:%d", bytesRecibidos);

		if(tipoMensajeConsulta == I_ESTADO_CLAVE) {
			valor = malloc(300);
			deserializar(respuestaConsulta, "%s", valor);
			log_info(logger, "Respuesta Consulta Instancia: %s", valor);
		} else if (tipoMensajeConsulta == DESCONEXION) {
			validarDesconexionDeInstancia(instancia->socket);
		}

		free(respuestaConsulta);
	} else {
		valor = strdup("Clave no existente");
	}

	//Respondo lo averiguado
	char *nombreInstancia = instancia->nombre;
	tPaquete pkgEstadoClave;
	pkgEstadoClave.type = C_ESTADO_CLAVE;
	pkgEstadoClave.length = serializar(pkgEstadoClave.payload, "%s%s", valor, nombreInstancia);
	log_info(logger,"Se envia estado de clave %s, valor %s, instancia %s", claveConsultada, valor, nombreInstancia);
	int bytesEnviados = enviarPaquete(socketPlanificador, &pkgEstadoClave, logger, "Se envia estado de clave");
	log_info(logger,"Se envian %d bytes\n", bytesEnviados);
	free(valor);
}

char *buscarClavesPorInstancia(char *nombreInstancia) {
	char *cadena = string_new();
	void agregarClaveDeInstancia(char *clave, t_instancia *instancia) {
		if (strcmp(nombreInstancia, instancia->nombre) == 0) {
			string_append_with_format(&cadena, "%s;", clave);
		}
	}
	dictionary_iterator(diccionarioClaves, buscarClavesPorInstancia);
	return cadena;
}

void ejecutarCompactacion() {
	//armo el set con los sockets de las instancias que tienen que compactar
	log_info(logger, "Se ejecuta compactacion");
	int socketMaximo = 0;
	fd_set setInstanciasCompactadoras;
	t_list *instanciasACompactar = list_create();
	FD_ZERO(&setInstanciasCompactadoras);
	void armarSetDeSockets(t_instancia *instancia) {
		//Para las instancias que no pertenezcan al ciclo de la operacion
		if (instancia->socket != instanciaElegida->socket && instancia->socket != -1) {
			FD_SET(instancia->socket, &setInstanciasCompactadoras);
			list_add(instanciasACompactar, instancia);
			if (instancia->socket > socketMaximo) {
				socketMaximo = instancia->socket;
			}
		}
	}
	list_iterate(instancias, armarSetDeSockets);

	//preparo el paquete de orden de compactacion
	char *ordenCompactacion = strdup("COMPACTAR");
	tPaquete pkgCompactacion;
	pkgCompactacion.type = C_EJECUTAR_COMPACTACION;
	pkgCompactacion.length = serializar(pkgCompactacion.payload, "%s", ordenCompactacion);

	//envio el paquete a las instancias involucradas
	int i;
	for (i = 0; i < instanciasACompactar->elements_count; i++) {
		t_instancia *instanciaReceptora = list_get(instanciasACompactar, i);
		enviarPaquete(instanciaReceptora->socket, &pkgCompactacion, logger, "Se envia orden de compactacion a instancia");
		log_info(logger, "Se envia a la instancia %s una orden de compactacion", instanciaReceptora->nombre);
	}
	free(ordenCompactacion);

	//funcion utilitaria
	void dejarDeEsperarInstancia(int socketInstancia) {
		int j;
		for (j = 0; j < instanciasACompactar->elements_count; j++) {
			t_instancia *instanciaARemover = list_get(instanciasACompactar, j);
			if (instanciaARemover->socket == socketInstancia) {
				list_remove(instanciasACompactar, j);
				return;
			}
		}
	}

	//espero respuestas
	fd_set temp;
	tMensaje tipoMensaje;
	char *buffer = malloc(100);
	while (instanciasACompactar->elements_count > 0) {
		int socketMultiplexado = multiplexar(&setInstanciasCompactadoras, &temp, &socketMaximo, &tipoMensaje, &buffer, logger);
		if (socketMultiplexado >= 0) {
			switch (tipoMensaje) {
			case I_COMPACTACION_TERMINADA:
				dejarDeEsperarInstancia(socketMultiplexado);
				break;
			case DESCONEXION:
				validarDesconexionDeInstancia(socketMultiplexado);
				dejarDeEsperarInstancia(socketMultiplexado);
				break;
			case I_RESULTADO_ERROR:
				dejarDeEsperarInstancia(socketMultiplexado);
				break;
			}
		}
	}
}
