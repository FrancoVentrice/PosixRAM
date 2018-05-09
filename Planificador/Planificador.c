/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Planificador.h"

pthread_mutex_t mutexESI;
void atenderESI(int* iSocketComunicacion) {

	//tArgs* my_data=malloc(sizeof(tArgs));
	int bytesEnviados;

	//my_data = (tArgs*) argumentos;
	puts("HANDSHAKE CON ESI");
	t_esi* esiNuevo = malloc(sizeof(t_esi));

	pthread_mutex_lock(&mutexESI);
	list_add(colaDeListos, esiNuevo);
	estimarHRRN();
	pthread_mutex_unlock(&mutexESI);

	if (esiEnEjecucion == NULL) { // SI NO HAY NINGUNO EJECUTANDO NOTIFICAR OK
		tRespuesta* respuestaEjecucion = malloc(sizeof(tRespuesta));
		respuestaEjecucion->mensaje = malloc(100);
		strcpy(respuestaEjecucion->mensaje, "OK");
		tPaquete pkgHandshakeRespuesta;
		pkgHandshakeRespuesta.type = P_HANDSHAKE;

		pkgHandshakeRespuesta.length = serializar(pkgHandshakeRespuesta.payload,
				"%c%s", pkgHandshakeRespuesta.type,
				respuestaEjecucion->mensaje);

		printf("Tipo: %d, largo: %d \n", pkgHandshakeRespuesta.type,
				pkgHandshakeRespuesta.length);

		puts("Se envia respuesta");
		bytesEnviados = enviarPaquete(*iSocketComunicacion,
				&pkgHandshakeRespuesta, logger, "Se envia respuesta a ESI");

		printf("Se envian %d bytes\n", bytesEnviados);

	} else {
		tRespuesta* respuestaRechazo = malloc(sizeof(tRespuesta));
		respuestaRechazo->mensaje = malloc(100);
		strcpy(respuestaRechazo->mensaje, "CPU OCUPADA");
		tPaquete pkgHandshakeRespuesta;
		pkgHandshakeRespuesta.type = P_HANDSHAKE;

		pkgHandshakeRespuesta.length = serializar(pkgHandshakeRespuesta.payload,
				"%c%s", pkgHandshakeRespuesta.type, respuestaRechazo->mensaje);

		printf("Tipo: %d, largo: %d \n", pkgHandshakeRespuesta.type,
				pkgHandshakeRespuesta.length);

		puts("Se envia respuesta");
		bytesEnviados = enviarPaquete(*iSocketComunicacion,
				&pkgHandshakeRespuesta, logger, "Se envia respuesta a ESI");

		printf("Se envian %d bytes\n", bytesEnviados);
	}

}

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	escucharESIs();
	levantarConsola();
	finalizar(0);
}

void finalizar(int codigo) {
	pthread_join(hiloConsola, NULL);
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
void escucharESIs() {
	fd_set master;
	fd_set read_fds;
	int *fdmax;
	int bytesEnviados;
	int maxSock;
	int iSocketEscucha;
	int iSocketComunicacion;
	int tamanioMensaje = 0;
	int idSiguienteEsi=0;
	t_esi* esiNuevo=malloc(sizeof(t_esi));
	pthread_t hiloESI;


	int puertoEscucha = configuracion->puerto;
	int puertoConexion = configuracion->puertoCoordinador;
	char* ipCoordinador = configuracion->ipCoordinador;

	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	//Conexion al Coordinador
	int socketCoordinador = connectToServer("127.0.0.1", puertoConexion,
			logger);
	tSolicitudPlanificador* solicitudPlanificador = malloc(
			sizeof(tSolicitudPlanificador));
	solicitudPlanificador->mensaje = malloc(100);
	strcpy(solicitudPlanificador->mensaje, "SOY PLANIFIFCADOR");
	tPaquete pkgHandshakeCoordinador;
	pkgHandshakeCoordinador.type = P_HANDSHAKE;
	int tamanioTotal = 0;
	tRespuesta *respuesta = malloc(sizeof(tRespuesta));

	pkgHandshakeCoordinador.length = serializar(pkgHandshakeCoordinador.payload,
			"%c%s", pkgHandshakeCoordinador.type,
			solicitudPlanificador->mensaje);

	puts("Se envia solicitud de ejecucion");
	tamanioTotal = enviarPaquete(socketCoordinador, &pkgHandshakeCoordinador,
			logger, "Se envia solicitud de ejecucion");
	printf("Se envian %d bytes\n", tamanioTotal);

	//RESPUESTA DEL COORDINADOR
	tMensaje tipoMensaje;
	char * sPayloadRespuestaHand = malloc(100);

	int bytesRecibidos = recibirPaquete(socketCoordinador, &tipoMensaje,
			&sPayloadRespuestaHand, logger, "Hand Respuesta");
	printf("RECIBIDOS:%d\n", bytesRecibidos);
	respuesta->mensaje = malloc(10);
	char encabezado_mensaje;

	deserializar(sPayloadRespuestaHand, "%c%s", &encabezado_mensaje,
			respuesta->mensaje);
	printf("RESPUESTA: %s \n", respuesta->mensaje);

	// Inicializacion de sockets y actualizacion del log
	iSocketEscucha = crearSocketEscucha(puertoEscucha, logger);

	FD_SET(iSocketEscucha, &setSocketsOrquestador);
	maxSock = iSocketEscucha;

	tPaquete pkgHandshake;
	char * sPayloadRespuesta = malloc(100);
	char encabezadoMensaje;
	tSolicitudESI *solicitud = malloc(sizeof(tSolicitudESI));
	solicitud->mensaje = malloc(100);
	int recibidos;
	puts("Escuchando");

	while (1) {
		iSocketComunicacion = getConnection(&setSocketsOrquestador, &maxSock,
				iSocketEscucha, &tipoMensaje, &sPayloadRespuesta, logger);

		if (iSocketComunicacion != -1) {
			switch (tipoMensaje) {
			case E_HANDSHAKE:
				//creo el hilo para el ESI que quiero atender

				pthread_create(&hiloESI,NULL,atenderESI,&iSocketComunicacion);
				pthread_join(hiloESI,NULL);


				//le envio el OK a ESI para ejecutar




				tipoMensaje = DESCONEXION;

				break;

			case DESCONEXION:
				break;
			}
		}
	}
}

void planificar() {
	switch (configuracion->algoritmoPlanificacion) {
	case ALGORITMO_SJF_CD:
	case ALGORITMO_SJF_SD:
		estimarSJF();
		break;
	case ALGORITMO_HRRN:
		estimarHRRN();
		break;
	}
}

void estimarHRRN() {
	t_esi* EsiHRRNMayor;
	alfa = (float) (configuracion->alfa) / (float) 100;

	float RRMayor = 0;
	int indexHRRNMayor = 0;
	int tiempoRespuesta = 0;
	float responseRatio = 0;

	int i;
	for (i = 0; i < colaDeListos->elements_count; i++) {
		t_esi* esiActual = list_get(colaDeListos, i);
		if (esiActual->rafagaAnterior != 0) {
			esiActual->estimacionAnterior = esiActual->estimacion;
			esiActual->estimacion = (float) alfa
					* (float) esiActual->rafagaAnterior
					+ (1 - alfa) * esiActual->estimacionAnterior;
			tiempoRespuesta = calcularTiempoRespuesta(esiActual);
			responseRatio = ((float) tiempoRespuesta
					+ (float) esiActual->estimacion)
					/ (float) esiActual->estimacion;

			if (responseRatio > RRMayor) {
				RRMayor = responseRatio;
				indexHRRNMayor = i;
			}

		}
	}
	EsiHRRNMayor = list_get(colaDeListos, indexHRRNMayor);
	if (!esiEnEjecucion) {
		esiEnEjecucion = EsiHRRNMayor;
		printf("EJECUTANDO:\n");
		list_remove(colaDeListos, indexHRRNMayor);
	} else {
		printf("La CPU se encuentra ocupada\n");
	}
}

int calcularTiempoRespuesta(t_esi* esi) {
	return tiempoTotalEjecucion - esi->instanteLlegadaAListos;
}



//se corre cuando hay que elegir un nuevo ESI a ejecutar
//si es con desalojo, tambien se corre cuando entra un nuevo proceso a la lista
//setea la estimacion de todos los ESIs en la cola de listos
//setea el proximo ESI a ejecutar
void estimarSJF() {
	//me hago una pasadita para volar los bloqueados
	list_remove_by_condition(colaDeListos, evaluarBloqueoDeEsi);
	if (esiEnEjecucion && esiEnEjecucion->bloqueado) {
		esiEnEjecucion = NULL;
	}

	if (colaDeListos->elements_count == 0) {
		return;
	}

	t_esi *ESIMasCorto = list_get(colaDeListos, 0);
	int indexDelESIMasCorto = 0;
	int i;
	float alfa = configuracion->alfa / 100;
	for (i = 0; i < colaDeListos->elements_count; i++) {
		t_esi *esi = list_get(colaDeListos, i);
		//rafagaAnterior lo uso como flag para saber si la estimacion esta actualizada
		if (esi->rafagaAnterior != 0) {
		esi->estimacion = alfa * esi->rafagaAnterior + (1 - alfa) * esi->estimacionAnterior;
		//"actualizo" la estimacion seteando en 0 la rafaga anterior, total ya no la voy a usar mas
		esi->rafagaAnterior = 0;
		//seteo la nueva estimacion tambien a estimacionAnterior, ya que estimacion
		//se va a ir decrementando con las ejecuciones, y estimacionAnterior queda estatica y
		//me sirve para la proxima vez que tenga que estimar
		esi->estimacionAnterior = esi->estimacion;
		}
		if (esi->estimacion < ESIMasCorto->estimacion) {
			ESIMasCorto = esi;
			indexDelESIMasCorto = i;
		}
	}

	//finalmente, comparo al mas corto de la cola de listos con el que ya estaba ejecutando
	//la prioridad en caso de igualdad la tiene el que ya estaba ejecutando
	//si habia uno en ejecucion, lo devuelvo a listos
	if (!esiEnEjecucion) {
		esiEnEjecucion = ESIMasCorto;
		list_remove(colaDeListos, indexDelESIMasCorto);
	} else if (ESIMasCorto->estimacion < esiEnEjecucion->estimacion) {
		list_add(colaDeListos, esiEnEjecucion);
		esiEnEjecucion = ESIMasCorto;
		list_remove(colaDeListos, indexDelESIMasCorto);
	}
}

void sentenciaEjecutadaCorrectamenteSJF() {
	esiEnEjecucion->rafagaAnterior ++;
	esiEnEjecucion->estimacion--;
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
}

//Cuando ESI hace un GET exitosamente
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
	if (dictionary_has_key(diccionarioBloqueados, clave)) {
		t_list *bloqueados = dictionary_get(diccionarioBloqueados, clave);
		int i;
		for (i = 0; i < bloqueados->elements_count; i++) {
			t_esi *esi = list_get(bloqueados, i);
			esi->bloqueado = false;
			list_add(colaDeListos, esi);
		}
		dictionary_remove(diccionarioBloqueados, clave);
		list_destroy(bloqueados);
	}
	if (dictionary_has_key(diccionarioClavesTomadas, clave)) {
		t_esi *esi = dictionary_remove(diccionarioClavesTomadas, clave);
		esiRemoverClaveTomada(esi, clave);
	}
	free(clave);
}

t_esi *esiNew() {
	t_esi *esi = malloc(sizeof(t_esi));
	esi->clavesTomadas = list_create();
	return esi;
}

void esiDestroyer(t_esi *esi) {
	list_destroy_and_destroy_elements(esi->clavesTomadas, clavesTomadasDestroyer);
	free(esi->id);
	free(esi);
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
	while (esiEnEjecucion->clavesTomadas->elements_count > 0) {
		char *clave = list_get(esiEnEjecucion->clavesTomadas, 0);
		liberarClave(clave);
	}
	list_add(colaDeFinalizados, esiEnEjecucion);
	esiEnEjecucion = NULL;
}

void ejecutarComandosConsola() {
	int i;
	while (bufferConsola->elements_count > 0) {
		t_instruccion_consola *instruccion = list_get(bufferConsola, 0);
		switch (instruccion->instruccion) {
		case INSTRUCCION_BLOQUEAR:
			bloquearEsiPorConsola(instruccion->primerParametro, instruccion->segundoParametro);
			break;
		case INSTRUCCION_DESBLOQUEAR:
			break;
		case INSTRUCCION_TERMINAR:
			break;
		case INSTRUCCION_DEADLOCK:
			break;
		}
		list_remove_and_destroy_element(bufferConsola, 0, instruccionDestroyer);
	}
}

void bloquearEsiPorConsola(char *clave, char *id) {
	t_esi * esi = buscarEsiNoBloqueadoPorId(id);
	if (esi) {
		bloquearESIConClave(esi, clave);
		//To Do: ver si cuando se libera la clave se libera al ESI
		esi->bloqueadoPorConsola = true;
	}
}
