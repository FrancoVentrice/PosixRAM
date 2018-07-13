/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Planificador.h"

void atenderESI(int iSocketComunicacion) {
	int bytesEnviados;

	log_info(logger, "HANDSHAKE CON ESI");
	t_esi* esiNuevo = esiNew(iSocketComunicacion);

	//lo agrego a listos y le aviso Ok de Handshake
	tPaquete pkgHandshakeRespuesta;
	pkgHandshakeRespuesta.type = P_HANDSHAKE;
	pkgHandshakeRespuesta.length = serializar(pkgHandshakeRespuesta.payload, "", NULL);
	bytesEnviados = enviarPaquete(esiNuevo->socket, &pkgHandshakeRespuesta, logger, "Se envia respuesta de handshake a ESI");
	log_debug(logger,"Se enviaron %d bytes",bytesEnviados);

	log_info(logger,"Se agrega ESI a cola de listos: %s con socket: %d", esiNuevo->id, esiNuevo->socket);
	agregarEsiAColaDeListos(esiNuevo);
	pthread_mutex_unlock(&mutexEspera);
}

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	realizarHandshakeCoordinador();
	inicializarSockets();
	levantarHiloEscuchaESIs();
	levantarConsola();
	cicloPrincipal();
	finalizar(EXIT_SUCCESS);
}

//metodo para ir testeando los algoritmos de forma aislada
void test() {
	log_info(logger, "agrego esi 1\n");
	agregarEsiAColaDeListos(esiNew(1));
	log_info(logger, "agrego esi 2\n");
	agregarEsiAColaDeListos(esiNew(2));
	log_info(logger, "agrego esi 3\n");
	agregarEsiAColaDeListos(esiNew(3));
	log_info(logger, "agrego esi 4\n");
	agregarEsiAColaDeListos(esiNew(4));
}

//METODO PRINCIPAL
//una vez que todas las conexiones estan hechas, y el planificador pueda empezar, se llama a este metodo
//representa un ciclo en el cual atiende los comandos de consola, planifica (si es necesario) y avisa al ESI que ejecute
void cicloPrincipal() {
	while(vivo) {
		pthread_mutex_lock(&mutexEspera);
		ejecutarComandosConsola();
		if (ejecutando && aptoEjecucion) {
			if (planificacionNecesaria) {
				planificar();
			}
			cicloDeSentencia();
		}
		evaluarNecesidadDeEspera();
	}
}

void cicloDeSentencia() {
	enviarOrdenDeEjecucion();
	bool sentenciaActiva = true;
	while (sentenciaActiva) {
		fd_set temp;
		tMensaje tipoMensaje;
		char *buffer;
		int socketMultiplexado = multiplexar(&setSockets, &temp, &maxSock, &tipoMensaje, &buffer, logger);
		if (socketMultiplexado >= 0) {
			switch (tipoMensaje) {
			case E_ESI_FINALIZADO:
				log_info(logger, "ESI %s finalizado", esiEnEjecucion->id);
				esiFinalizado();
				sentenciaActiva = false;
				break;
			case E_LINEA_OK:
				break;
			case C_CONSULTA_OPERACION_GET:
			case C_CONSULTA_OPERACION_SET:
			case C_CONSULTA_OPERACION_STORE:
				recibirConsultaOperacion(tipoMensaje, buffer);
				evaluarConsultaDeOperacion();
				break;
			case C_RESULTADO_OPERACION:
				recibirResultadoOperacion(buffer);
				sentenciaActiva = false;
				break;
			case DESCONEXION:
				esiDesconectado(socketMultiplexado);
				if (!esiEnEjecucion)
					sentenciaActiva = false;
				break;
			default:
				log_warning(logger,"Se recibió un tipo de mensaje no esperado: %d", tipoMensaje);
				break;
			}
		}
	}
}

void evaluarNecesidadDeEspera() {
	bool parado = false;
	evaluarAptoEjecucion();
	parado = !(ejecutando && aptoEjecucion);
	if (!parado) {
		pthread_mutex_unlock(&mutexEspera);
	}
}

//representa un ciclo de ejecucion
//(avisar al ESI a ejecutar, esperar consulta, evaluar, y esperar fin de ejecucion de operacion)
void enviarOrdenDeEjecucion() {
	tPaquete pkgEjecutarLinea;
	pkgEjecutarLinea.type = P_EJECUTAR_LINEA;
	pkgEjecutarLinea.length = serializar(pkgEjecutarLinea.payload, "", NULL);
	log_info(logger, "Se envia orden para ejecutar al ESI: %s", esiEnEjecucion->id);
	int bytesEnviados = enviarPaquete(esiEnEjecucion->socket, &pkgEjecutarLinea, logger, "Se envia orden para ejecutar");
	log_debug(logger,"Se enviaron %d bytes",bytesEnviados);
}

void enviarOrdenDeAborcion(int socket) {
	tPaquete pkgAbortar;

	pkgAbortar.type = P_ABORTAR;
	pkgAbortar.length = serializar(pkgAbortar.payload, "", NULL);

	log_info(logger, "Se envia orden para abortar al ESI");

	int bytesEnviados = enviarPaquete(socket, &pkgAbortar, logger, "Se envia orden para abortar");
	log_debug(logger,"Se enviaron %d bytes",bytesEnviados);
}

void recibirConsultaOperacion(tMensaje tipoMensaje, char *sPayloadConsulta) {
	deserializar(sPayloadConsulta, "%s", consultaCoordinador->clave);
	switch(tipoMensaje) {

	case C_CONSULTA_OPERACION_GET:
		log_info(logger,"Consulta operacion GET para clave %s", consultaCoordinador->clave);
		consultaCoordinador->operacion = OPERACION_GET;
		break;

	case C_CONSULTA_OPERACION_SET:
		log_info(logger,"Consulta operacion SET para clave %s", consultaCoordinador->clave);
		consultaCoordinador->operacion = OPERACION_SET;
		break;

	case C_CONSULTA_OPERACION_STORE:
		consultaCoordinador->operacion = OPERACION_STORE;
		log_info(logger,"Consulta operacion STORE para clave %s", consultaCoordinador->clave);
		break;
	default:
		log_warning(logger,"Se recibió un tipo de mensaje no esperado: %d", tipoMensaje);
		break;
	}
}

void finalizar(int codigo) {
	log_info(logger, "Finalizando Planificador. Buenas noches :)");
	pthread_join(hiloConsola, NULL);
	pthread_cancel(hiloEscuchaESIs);
	limpiarConfiguracion();
	exit(codigo);
}

void levantarConsola() {
	int respHilo = 0;
	respHilo = pthread_create(&hiloConsola, NULL, consola, NULL);
	if (respHilo) {
		log_error(logger, "Error al levantar la consola");
		finalizar(EXIT_FAILURE);
	}
}

void realizarHandshakeCoordinador() {
	//Conexion al Coordinador
	socketCoordinador = connectToServer(configuracion->ipCoordinador, configuracion->puertoCoordinador, logger);
	tPaquete pkgHandshakeCoordinador;
	pkgHandshakeCoordinador.type = P_HANDSHAKE;
	pkgHandshakeCoordinador.length = serializar(pkgHandshakeCoordinador.payload, "", NULL);
	log_info(logger, "Se envia solicitud de handshake con coordinador");
	int bytesEnviados = enviarPaquete(socketCoordinador, &pkgHandshakeCoordinador, logger, "Se envia solicitud de handshake con coordinador");
	log_debug(logger,"Se enviaron %d bytes",bytesEnviados);

	//Respuesta del Coordinador
	tMensaje tipoMensaje;
	char *payloadRespuestaHand;
	int bytesRecibidos = recibirPaquete(socketCoordinador, &tipoMensaje, &payloadRespuestaHand, logger, "Hand Respuesta");
	log_debug(logger,"Se recibieron %d bytes",bytesRecibidos);
	if (tipoMensaje == C_HANDSHAKE) {
		log_info(logger, "Handshake con Coordinador exitoso\n");
	}
}

void inicializarSockets() {
	socketEscucha = crearSocketEscucha(configuracion->puerto, logger);
	FD_ZERO(&setSockets);
	//agrego el socket de coordinador al set de sockets
	FD_SET(socketCoordinador, &setSockets);
	maxSock = socketCoordinador;
}

void levantarHiloEscuchaESIs() {
	if (pthread_create(&hiloEscuchaESIs, NULL, escucharConexionesESIs, NULL)) {
		log_error(logger, "Error al levantar el hilo de escucha de ESIs");
		finalizar(EXIT_FAILURE);
	}
}

void escucharConexionesESIs() {
	while (1) {
		int iSocketComunicacion = getNewConnection(socketEscucha, &setSockets, &maxSock);
		if (iSocketComunicacion > 0) {
			atenderESI(iSocketComunicacion);
		}
	}
}

void recibirResultadoOperacion(char *bufferResultado) {
	tRespuestaCoordinador* respuestaCoordinador = malloc(sizeof(tRespuestaCoordinador));
	respuestaCoordinador->mensaje = malloc(100);
	deserializar(bufferResultado, "%s", respuestaCoordinador->mensaje);

	log_info(logger, "RESPUESTA OPERACION DEL COORDINADOR : %s",
			respuestaCoordinador->mensaje);
	if (strcmp(respuestaCoordinador->mensaje, "OK") == 0) {
		log_info(logger, "Sentencia finalizada: saxeeees!\n");
	} else if (strcmp(respuestaCoordinador->mensaje, "BLOQUEADO") == 0) {
		log_info(logger, "Sentencia finalizada: BAN HAMMER!!\n");
	} else if (strcmp(respuestaCoordinador->mensaje, "ERROR") == 0) {
		log_info(logger, "Sentencia finalizada: error :(\n");
		abortarEsiPorId(esiEnEjecucion->id);
	}
}

void enviarOperacionValida() {
	tPaquete pkgOperacionValida;
	int enviados;
	char* respuesta = malloc(10);
	strcpy(respuesta,"OK");
	pkgOperacionValida.type = P_RESPUESTA_CONSULTA;

	pkgOperacionValida.length = serializar(pkgOperacionValida.payload,"%s",respuesta);

	log_info(logger, "Se envia respuesta consulta OK ESI %s", esiEnEjecucion->id);
	enviados = enviarPaquete(socketCoordinador, &pkgOperacionValida,logger, "Se envia respuesta consulta");
	log_debug(logger,"Se enviaron %d bytes",enviados);
	free(respuesta);
}

void enviarOperacionInvalidaBloqueo() {
	tPaquete pkgOperacionValida;
	int enviados;
	char* respuesta = malloc(15);
	strcpy(respuesta,"BLOQUEADO");
	pkgOperacionValida.type = P_RESPUESTA_CONSULTA;

	pkgOperacionValida.length = serializar(pkgOperacionValida.payload,"%s",respuesta);

	log_info(logger, "Se envia respuesta consulta BLOQUEAR ESI %s", esiEnEjecucion->id);
	enviados = enviarPaquete(socketCoordinador, &pkgOperacionValida,logger, "Se envia respuesta consulta");
	log_debug(logger,"Se enviaron %d bytes",enviados);
	free(respuesta);
}

void enviarOperacionInvalidaError() {
	tPaquete pkgOperacionValida;
	int enviados;
	char* respuesta = malloc(15);
	strcpy(respuesta,"ERROR");
	pkgOperacionValida.type = P_RESPUESTA_CONSULTA;

	pkgOperacionValida.length = serializar(pkgOperacionValida.payload,"%s",respuesta);

	log_info(logger, "Se envia respuesta consulta ERROR ESI %s", esiEnEjecucion->id);
	enviados = enviarPaquete(socketCoordinador, &pkgOperacionValida,logger, "Se envia respuesta consulta");
	log_debug(logger,"Se enviaron %d bytes",enviados);
	free(respuesta);
}

void evaluarConsultaDeOperacion() {
	/*
	 * ToDo: actualizar logica de envio de mensaje,
	 * es un placeholder
	 *
	 * */
	char *clave = strdup(consultaCoordinador->clave);
	int operacion = consultaCoordinador->operacion;
	sentenciaEjecutada();
	switch (operacion) {
	case OPERACION_GET:
		if (evaluarBloqueoDeClave(clave)) {
			t_esi *esi = dictionary_get(diccionarioClavesTomadas, clave);
			if (esi != NULL && strcmp(esiEnEjecucion->id, esi->id) == 0) {
				enviarOperacionValida();
			} else {
				enviarOperacionInvalidaBloqueo();
				bloquearESIConClave(esiEnEjecucion, clave);
				log_info(logger, "El ESI en ejecucion esta siendo bloqueado por hacer un GET de la clave %s tomada", clave);
				planificacionNecesaria = true;
			}
		} else {
			enviarOperacionValida();
			esiTomaClave(esiEnEjecucion, clave);
		}
		break;
	case OPERACION_SET:
		if (evaluarBloqueoDeClave(clave)) {
			t_esi *esi = dictionary_get(diccionarioClavesTomadas, clave);
			if (esi != NULL && strcmp(esiEnEjecucion->id, esi->id) == 0) {
				enviarOperacionValida();
			} else {
				enviarOperacionInvalidaError();
			}
		} else {
			enviarOperacionInvalidaError();
		}
		break;
	case OPERACION_STORE:
		if (evaluarBloqueoDeClave(clave)) {
			t_esi *esi = dictionary_get(diccionarioClavesTomadas, clave);
			if (esi != NULL && strcmp(esiEnEjecucion->id, esi->id) == 0) {
				enviarOperacionValida();
				liberarClave(clave);
			} else {
				enviarOperacionInvalidaError();
			}
		} else {
			enviarOperacionInvalidaError();
		}
		break;
	}
}

void agregarEsiAColaDeListos(t_esi *esi) {
	pthread_mutex_lock(&mutexColaDeListos);
	bool previamenteVacia = colaDeListos->elements_count == 0 && esiEnEjecucion == NULL;
	list_add(colaDeListos, esi);
	pthread_mutex_unlock(&mutexColaDeListos);
	if (configuracion->algoritmoPlanificacion == ALGORITMO_SJF_CD) {
		planificacionNecesaria = true;
	} else if (configuracion->algoritmoPlanificacion == ALGORITMO_HRRN) {
		esi->instanteLlegadaAListos = tiempoTotalEjecucion;
	}
	//si no habia ESIs participando en el sistema, o los que habia estaban
	//bloqueados, sera necesaria una planificacion
	if (previamenteVacia) {
		planificacionNecesaria = true;
	}
	//re/afirma la capacidad del planificador de enviar ordenes de ejecucion
	aptoEjecucion = true;
	//Si entra otro ESI entonces se queda en cola de listos esperando el ok del planificador
}

void planificar() {
	log_info(logger, "Planifico: Habia un ESI en ejecucion?: %d", (bool) esiEnEjecucion);
	if (esiEnEjecucion) {
		log_info(logger, "ESI en ejecucion antes de planificar: %s", esiEnEjecucion->id);
	}
	//me hago una pasadita para volar los bloqueados
	pthread_mutex_lock(&mutexColaDeListos);
	list_remove_by_condition(colaDeListos, evaluarBloqueoDeEsi);
	pthread_mutex_unlock(&mutexColaDeListos);
	if (esiEnEjecucion && esiEnEjecucion->bloqueado) {
		esiEnEjecucion = NULL;
	}

	if (colaDeListos->elements_count == 0) {
		return;
	}

	int indexProximoAEjecutar;
	//estimo segun configuracion
	switch (configuracion->algoritmoPlanificacion) {
	case ALGORITMO_SJF_CD:
	case ALGORITMO_SJF_SD:
		indexProximoAEjecutar = planificarSJF();
		break;
	case ALGORITMO_HRRN:
		indexProximoAEjecutar = planificarHRRN();
		break;
	}

	//si es < 0, corresponde que siga ejecutando el que estaba
	if (indexProximoAEjecutar < 0) {
		log_info(logger,"Sigue ejecutando el mismo ESI");
		planificacionNecesaria = false;
		return;
	}

	//si ya habia uno en ejecucion, lo devuelvo a la cola de listos
	if (esiEnEjecucion) {
		list_add(colaDeListos, esiEnEjecucion);
	}

	esiEnEjecucion = list_remove(colaDeListos, indexProximoAEjecutar);
	log_info(logger,"Proximo ESI a ejecutar: %s con socket: %d", esiEnEjecucion->id, esiEnEjecucion->socket);
	planificacionNecesaria = false;
}

void sentenciaEjecutada() {
	if (esiEnEjecucion != NULL) {
		esiEnEjecucion->rafagaAnterior ++;
		esiEnEjecucion->estimacion--;
	}
	tiempoTotalEjecucion ++;
}

void esiFinalizado() {
	log_info(logger, "Finalizando ESI %s", esiEnEjecucion->id);
	finalizarEsiEnEjecucion();
	planificacionNecesaria = true;
}

int planificarHRRN() {
	float RRMayor = 0.0f;
	int indexHRRNMayor = 0;

	int i;
	for (i = 0; i < colaDeListos->elements_count; i++) {
		t_esi* esiActual = list_get(colaDeListos, i);
		int tiempoEspera = calcularTiempoEspera(esiActual);
		float responseRatio = 0.0f;
		responseRatio = ((float) tiempoEspera + (float) esiActual->estimacion) / (float) esiActual->estimacion;
		log_info(logger, "\nNombre: %s\nWait: %d\nEstimacion: %.2f\nResponse ratio: %.2f", esiActual->id, tiempoEspera, esiActual->estimacion, responseRatio);

		if (responseRatio > RRMayor) {
			RRMayor = responseRatio;
			indexHRRNMayor = i;
		}
	}
	return indexHRRNMayor;
}

int calcularTiempoEspera(t_esi* esi) {
	return tiempoTotalEjecucion - esi->instanteLlegadaAListos;
}

//se corre cuando hay que elegir un nuevo ESI a ejecutar
//elige el proximo ESI a ejecutar
int planificarSJF() {
	t_esi *esiMasCorto = list_get(colaDeListos, 0);
	int indexDelESIMasCorto = 0;
	int i;
	for (i = 0; i < colaDeListos->elements_count; i++) {
		t_esi *esi = list_get(colaDeListos, i);
		if (esi->estimacion < esiMasCorto->estimacion) {
			esiMasCorto = esi;
			indexDelESIMasCorto = i;
		}
	}

	//si es con desalojo, tengo en cuenta al esi en ejecucion
	//si no es con desalojo, entonces no hizo falta llamar a este metodo
	if (configuracion->algoritmoPlanificacion == ALGORITMO_SJF_CD
			&& esiEnEjecucion != NULL
			&& esiMasCorto->estimacion >= esiEnEjecucion->estimacion) {
		indexDelESIMasCorto = -1;
	}

	return indexDelESIMasCorto;
}

void estimar(t_esi *esi) {
	esi->estimacion = configuracion->alfa * esi->rafagaAnterior + (1 - configuracion->alfa) * esi->estimacionAnterior;
	//"actualizo" la estimacion seteando en 0 la rafaga anterior, total ya no la voy a usar mas
	esi->rafagaAnterior = 0;
	//seteo la nueva estimacion tambien a estimacionAnterior, ya que estimacion
	//se va a ir decrementando con las ejecuciones, y estimacionAnterior queda estatica y
	//me sirve para la proxima vez que tenga que estimar
	esi->estimacionAnterior = esi->estimacion;
	log_info(logger, "Estimacion de %s es %.2f\n", esi->id, esi->estimacion);
}

//Cuando ESI hace un GET y la clave esta tomada
void bloquearESIConClave(t_esi *esi, char *clave) {
	esi->bloqueado = true;
	if (!dictionary_has_key(diccionarioBloqueados, clave) ||
			!dictionary_has_key(diccionarioClavesTomadas, clave)) {
		bloquearClaveSola(clave);
	}
	t_list *bloqueados = dictionary_get(diccionarioBloqueados, clave);
	list_add(bloqueados, esi);
	if (esiEnEjecucion != NULL && strcmp(esi->id, esiEnEjecucion->id) == 0) {
		esiEnEjecucion = NULL;
		planificacionNecesaria = true;
	}
}

//Cuando ESI hace un GET exitosamente
//Se valida que exista la cola de bloqueados correspondiente
//Se agrega/modifica el ESI que la tomo
//Se agrega la clave entre las tomadas del ESI
void esiTomaClave(t_esi *esi, char *clave) {
	if (!dictionary_has_key(diccionarioBloqueados, clave)) {
		t_list *nuevaCola = list_create();
		dictionary_put(diccionarioBloqueados, clave, nuevaCola);
	}
	if (dictionary_has_key(diccionarioClavesTomadas, clave)) {
		dictionary_remove(diccionarioClavesTomadas, clave);
	}
	dictionary_put(diccionarioClavesTomadas, clave, esi);
	list_add(esi->clavesTomadas, clave);
}

void bloquearClaveSola(char *clave) {
	if (!dictionary_has_key(diccionarioBloqueados, clave)) {
		t_list *nuevaCola = list_create();
		dictionary_put(diccionarioBloqueados, clave, nuevaCola);
	}
	if (!dictionary_has_key(diccionarioClavesTomadas, clave)) {
		dictionary_put(diccionarioClavesTomadas, clave, NULL);
	}
}

void liberarClave(char *clave) {
	liberarPrimerProcesoBloqueado(clave);
	if (dictionary_has_key(diccionarioClavesTomadas, clave)) {
		t_esi *esi = dictionary_remove(diccionarioClavesTomadas, clave);
		if (esi != NULL) {
			esiRemoverClaveTomada(esi, clave);
		}
	}
}

void liberarClavesDeEsi(t_esi *esi) {
	while (esi->clavesTomadas->elements_count > 0) {
		char *clave = list_get(esi->clavesTomadas, 0);
		liberarClave(clave);
		free(clave);
	}
}

void liberarPrimerProcesoBloqueado(char *clave) {
	if (dictionary_has_key(diccionarioBloqueados, clave)) {
		t_list *bloqueados = dictionary_get(diccionarioBloqueados, clave);
		if (bloqueados->elements_count > 0) {
			t_esi *esi = list_remove(bloqueados, 0);
			esi->bloqueado = false;
			estimar(esi);
			agregarEsiAColaDeListos(esi);
		}
		if (bloqueados->elements_count == 0) {
			dictionary_remove_and_destroy(diccionarioBloqueados, clave, list_destroy);
		}
	}
}

t_esi *esiNew(int socket) {
	nId++;
	t_esi *esi = malloc(sizeof(t_esi));
	esi->id = string_new();
	string_append_with_format(&(esi->id), "%s%d", "ESI", nId);
	esi->clavesTomadas = list_create();
	esi->estimacion = (float) configuracion->estimacionInicial;
	esi->estimacionAnterior = (float) configuracion->estimacionInicial;
	esi->socket = socket;
	esi->bloqueado = false;
	esi->rafagaAnterior = 0;
	list_add(esisExistentes, esi);
	return esi;
}

void esiDestroyer(t_esi *esi) {
	if (esi != NULL) {
		list_destroy_and_destroy_elements(esi->clavesTomadas, clavesTomadasDestroyer);
		free(esi->id);
		free(esi);
	}
}

void esiListDestroyer(t_list *esis) {
	list_destroy_and_destroy_elements(esis, esiDestroyer);
}

bool evaluarBloqueoDeEsi(t_esi *esi) {
	return esi->bloqueado;
}

bool evaluarBloqueoDeClave(char *clave) {
	return dictionary_has_key(diccionarioClavesTomadas, clave);
}

void esiRemoverClaveTomada(t_esi *esi, char *clave) {
	int i;
	for (i = 0; i < esi->clavesTomadas->elements_count; i++) {
		char *clavei = list_get(esi->clavesTomadas, i);
		if (strcmp(clave, clavei) == 0) {
			list_remove(esi->clavesTomadas, i);
			return;
		}
	}
}

t_esi * buscarEsiNoBloqueadoPorId(char *id) {
	int i;
	for (i = 0; i < colaDeListos->elements_count; i++) {
		t_esi * esi = list_get(colaDeListos, i);
		if (strcmp(esi->id, id) == 0) {
			if (esi->bloqueado) {
				log_info(logger, "\nEl ESI buscado está bloqueado\n");
				return NULL;
			} else {
				return esi;
			}
		}
	}
	if (strcmp(esiEnEjecucion->id, id) == 0) {
		if (esiEnEjecucion->bloqueado) {
			log_info(logger, "\nEl ESI buscado está bloqueado\n");
			return NULL;
		} else {
			return esiEnEjecucion;
		}
	}
	log_info(logger, "\nEl ESI buscado no existe o está bloqueado\n");
	return NULL;
}

void clavesTomadasDestroyer(char *clave) {
	free(clave);
}

void finalizarEsiEnEjecucion() {
	log_info(logger, "finalizando esi en ejecucion: %s\n", esiEnEjecucion->id);
	liberarClavesDeEsi(esiEnEjecucion);
	list_add(colaDeFinalizados, esiEnEjecucion);
	FD_CLR(esiEnEjecucion->socket, &setSockets);
	esiEnEjecucion->socket = -1;
	esiEnEjecucion = NULL;
	evaluarAptoEjecucion();
}

void ejecutarComandosConsola() {
	while (bufferConsola->elements_count > 0) {
		t_instruccion_consola *instruccion = list_get(bufferConsola, 0);
		switch (instruccion->instruccion) {
		case INSTRUCCION_BLOQUEAR:
			log_info(logger, "Instruccion bloquear\n");
			bloquearEsiPorConsola(instruccion->primerParametro, instruccion->segundoParametro);
			break;
		case INSTRUCCION_DESBLOQUEAR:
			log_info(logger, "Instruccion desbloquear\n");
			liberarPrimerProcesoBloqueado(instruccion->primerParametro);
			if (!dictionary_has_key(diccionarioBloqueados, instruccion->primerParametro)) {
				liberarClave(instruccion->primerParametro);
			}
			break;
		case INSTRUCCION_TERMINAR:
			log_info(logger, "Instruccion terminar\n");
			abortarEsiPorId(instruccion->primerParametro);
			break;
		case INSTRUCCION_DEADLOCK:
			log_info(logger, "Instruccion deadlock\n");
			analizarDeadlock();
			break;
		case INSTRUCCION_LISTAR:
			log_info(logger, "Instruccion listar\n");
			listarEsisPorRecurso(instruccion->primerParametro);
			break;
		case INSTRUCCION_STATUS:
			log_info(logger, "Instruccion status\n");
			statusDeClave(instruccion->primerParametro);
			break;
		}
		list_remove_and_destroy_element(bufferConsola, 0, instruccionDestroyer);
	}
}

void bloquearEsiPorConsola(char *clave, char *id) {
	t_esi * esi = buscarEsiNoBloqueadoPorId(id);
	if (esi) {
		bloquearESIConClave(esi, clave);
	}
}

void abortarEsiPorId(char *id) {
	log_info(logger, "Abortando ESI %s", id);
	t_esi *esi = NULL;
	if (esiEnEjecucion != NULL && strcmp(id, esiEnEjecucion->id) == 0) {
		esi = esiEnEjecucion;
		liberarClavesDeEsi(esiEnEjecucion);
		planificacionNecesaria = true;
		esiEnEjecucion = NULL;
	} else if ((esi = encontrarEsiPorId(colaDeListos, id)) != NULL) {
		log_info(logger, "Esi %s a abortar encontrado en cola de listos", id);
		liberarClavesDeEsi(esi);
		list_remove(colaDeListos, getIndexDeEsi(colaDeListos, esi));
	} else {
		void abortarEsiEnDiccionarioBloqueados(char *clave, t_list *bloqueados) {
			t_esi *iesi = encontrarEsiPorId(bloqueados, id);
			if (iesi != NULL) {
				log_info(logger, "Esi %s a abortar encontrado en cola de bloqueados de clave %s", id, clave);
				esi = iesi;
				liberarClavesDeEsi(esi);
				list_remove(bloqueados, getIndexDeEsi(bloqueados, esi));
			}
		}
		dictionary_iterator(diccionarioBloqueados, abortarEsiEnDiccionarioBloqueados);
	}
	if (esi != NULL) {
		int index = getIndexDeEsi(esisExistentes, esi);
		if (index != -1) {
			enviarOrdenDeAborcion(esi->socket);
			FD_CLR(esi->socket, &setSockets);
			esi->socket = -1;
			list_remove_and_destroy_element(esisExistentes, index, esiDestroyer);
		}
	} else {
		log_info(logger, "Esi %s no encontrado");
	}
	evaluarAptoEjecucion();
}

t_esi * encontrarEsiPorId(t_list *lista, char *id) {
	bool matcheaId(t_esi *esi) {
		return strcmp(esi->id, id) == 0;
	}
	return list_find(lista, matcheaId);
}

int getIndexDeEsi(t_list *lista, t_esi *esi) {
	int i;
	for (i = 0; i < lista->elements_count; i++) {
		t_esi *iesi = list_get(lista, i);
		if (strcmp(esi->id, iesi->id) == 0) {
			return i;
		}
	}
	return -1;
}

t_esi * encontrarEsiPorSocket(t_list *lista, int socket) {
	bool matcheaSocket(t_esi *esi) {
		return esi->socket == socket;
	}
	return list_find(lista, matcheaSocket);
}

void esiDesconectado(int socket) {
	log_info(logger, "Abortando ESI de socket %d", socket);
	t_esi *esi = NULL;
	if (socket == esiEnEjecucion->socket) {
		esi = esiEnEjecucion;
		log_info(logger, "Encontrado, esi en ejecucion va a ser abortado");
		liberarClavesDeEsi(esiEnEjecucion);
		planificacionNecesaria = true;
		esiEnEjecucion = NULL;
	} else if ((esi = encontrarEsiPorSocket(colaDeListos, socket)) != NULL) {
		liberarClavesDeEsi(esi);
		list_remove(colaDeListos, getIndexDeEsi(colaDeListos, esi));
	} else {
	void abortarEsiEnDiccionarioBloqueados(char *clave, t_list *bloqueados) {
		t_esi *iesi = encontrarEsiPorSocket(bloqueados, socket);
		if (iesi != NULL) {
			log_info(logger, "Esi %s a abortar encontrado en cola de bloqueados de clave %s", iesi->id, clave);
			esi = iesi;
			liberarClavesDeEsi(esi);
			list_remove(bloqueados, getIndexDeEsi(bloqueados, esi));
		}
	}
	dictionary_iterator(diccionarioBloqueados, abortarEsiEnDiccionarioBloqueados);
	}
	int index = getIndexDeEsi(esisExistentes, esi);
	if (index != -1) {
		FD_CLR(socket, &setSockets);
		esi->socket = -1;
		list_remove_and_destroy_element(esisExistentes, getIndexDeEsi(esisExistentes, esi), esiDestroyer);
	}
	evaluarAptoEjecucion();
}

void analizarDeadlock() {
	//esta funcion hace el analisis del deadlock por una entrada en el diccionario de bloqueados
	//se usa con la funcion de iteracion de diccionario sobre el diccionario de bloqueados
	void analizarDiccionarioBloqueados(char *clave, t_list *bloqueados) {
		if (bloqueados == NULL || bloqueados->elements_count == 0) {
			return;
		}
		//esta lista se va guardando los implicados en el posible deadlock
		t_list * empernados = list_create();
		char *claveABuscar = clave;
		bool buscando = true;
		while (buscando) {
			if (dictionary_has_key(diccionarioClavesTomadas, claveABuscar)) {
				t_esi *tomadorDeClave = NULL;
				tomadorDeClave = dictionary_get(diccionarioClavesTomadas, claveABuscar);
				//el deadlock va a ser posible si el tomador de la clave esta bloqueado (y la clave esta tomada por un esi)
				if (tomadorDeClave != NULL && tomadorDeClave->bloqueado) {
					//si no lo tengo en la lista de empernados, es porque no llegue a una dependencia circular
					if (getIndexDeEsi(empernados, tomadorDeClave) < 0) {
						list_add(empernados, tomadorDeClave);
						char *claveQueLoTieneBloqueado = NULL;
						//nombre feo si los hay, pero descriptivo
						void buscarClaveQueLoTieneBloqueadoAlEsi(char *clave, t_list *bloqueados) {
							if (getIndexDeEsi(bloqueados, tomadorDeClave) >= 0) {
								claveQueLoTieneBloqueado = clave;
							}
						}
						dictionary_iterator(diccionarioBloqueados, buscarClaveQueLoTieneBloqueadoAlEsi);
						if (claveQueLoTieneBloqueado != NULL) {
							claveABuscar = claveQueLoTieneBloqueado;
						}
					} else  {
						//llegue a una dependencia circular! todos los empernados son los participantes de esta barbarie
						log_error(logger, "\nDEADLOCK ENCONTRADO\n");
						log_info(logger, "\nRecurso en disputa: %s", clave);
						log_info(logger, "\nLos ESIs implicados son los siguientes:");
						int i;
						for (i = 0; i < empernados->elements_count; i++) {
							char *registro = string_new();
							t_esi *empernado = list_get(empernados, i);
							string_append_with_format(&registro, "\nNombre: %s\nClaves:", empernado->id);
							int j;
							for (j = 0; j < empernado->clavesTomadas->elements_count; j++) {
								string_append_with_format(&registro, "\n- %s ", list_get(empernado->clavesTomadas, j));
							}
							log_info(logger, registro);
						}
						buscando = false;
					}
				} else {
					buscando = false;
				}
			}
		}
		list_destroy(empernados);
	}
	//con esta iteracion analizo todas las claves del diccionario de bloqueados
	dictionary_iterator(diccionarioBloqueados, analizarDiccionarioBloqueados);
}

void listarEsisPorRecurso(char *clave) {
	log_info(logger, "Listando ESIs bloqueados por el recurso %s:\n", clave);
	if (dictionary_has_key(diccionarioBloqueados, clave)) {
		t_list *esis = dictionary_get(diccionarioBloqueados, clave);
		if (esis->elements_count == 0) {
			log_info(logger, "No hay ESIs bloqueados por la clave\n");
		} else {
			int i;
			for (i = 0; i < esis->elements_count; i++) {
				t_esi *iesi = list_get(esis, i);
				log_info(logger, "%s\n", iesi->id);
			}
		}
	} else {
		log_info(logger, "No hay ESIs bloqueados por la clave\n");
	}
}

void statusDeClave(char *clave) {
	log_info(logger, "Status de clave: %s", clave);

	//Se envia consulta de estado de clave al coordinador
	tPaquete pkgConsultaClave;
	pkgConsultaClave.type = P_ESTADO_CLAVE;
	pkgConsultaClave.length = serializar(pkgConsultaClave.payload, "%s", clave);
	log_info(logger, "Se envia consulta de clave %s al coordinador", clave);
	int enviados = enviarPaquete(socketCoordinador, &pkgConsultaClave, logger, "Se envia consulta de clave al coordinador");
	log_debug(logger,"Se enviaron %d bytes",enviados);

	//Se recibe respuesta de consulta del coordinador
	tMensaje tipoMensaje;
	char *sPayloadRespuestaPlanificador;
	log_info(logger, "Esperando respuesta de consulta por clave %s", clave);
	int bytesRecibidos = recibirPaquete(socketCoordinador, &tipoMensaje, &sPayloadRespuestaPlanificador, logger, "Esperando respuesta de consulta");
	log_info(logger, "RECIBIDOS:%d", bytesRecibidos);
	char *valor = malloc(bytesRecibidos);
	char *instanciaActual = malloc(40);
	deserializar(sPayloadRespuestaPlanificador, "%s%s", valor, instanciaActual);
	log_info(logger, "\nESTADO DE CLAVE\nclave: %s\nvalor: %s\ninstancia actual/probable: %s", clave, valor, instanciaActual);
	listarEsisPorRecurso(clave);

	//Liberamos la memoria asignada
	free(sPayloadRespuestaPlanificador);
	free(valor);
	free(instanciaActual);
}

void evaluarAptoEjecucion() {
	aptoEjecucion = !((esiEnEjecucion == NULL || esiEnEjecucion->bloqueado)
			&& (colaDeListos->elements_count == 0 || list_all_satisfy(colaDeListos, evaluarBloqueoDeEsi)));
}
