/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Planificador.h"

void atenderESI(int* iSocketComunicacion) {

	//tArgs* my_data=malloc(sizeof(tArgs));
	int bytesEnviados;

	//my_data = (tArgs*) argumentos;
	puts("HANDSHAKE CON ESI");
	t_esi* esiNuevo = esiNew(iSocketComunicacion);

	if (esiEnEjecucion == NULL) { // SI NO HAY NINGUNO EJECUTANDO NOTIFICAR OK

		//lo agrego a listos y planifico
		agregarEsiAColaDeListos(esiNuevo);

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

	} else {//Si no, lo agrego a listos solamente y espero a que se libere la CPU para replanificar
		agregarEsiAColaDeListos(esiNuevo);
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
}

//METODO PRINCIPAL
//una vez que todas las conexiones estan hechas, y el planificador pueda empezar, se llama a este metodo
//representa un ciclo en el cual atiende los comandos de consola, planifica (si es necesario) y avisa al ESI que ejecute
//se llama cuando una sentencia del ESI fue ejecutada correctamente (mas alla del resultado)
//se llama cuando la cola de listos vacia se popula
void trabajar() {
	if (ejecutando) {
		ejecutarComandosConsola();
		if (planificacionNecesaria) {
			planificar();
		}
		//insertar metodo en el que se avisa al ESI que ejecute una linea
	}
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
	int socketCoordinador = connectToServer(ipCoordinador, puertoConexion,
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

void agregarEsiAColaDeListos(t_esi *esi) {
	pthread_mutex_lock(&mutexColaDeListos);
	bool previamenteVacia = colaDeListos->elements_count == 0;
	list_add(colaDeListos, esi);
	pthread_mutex_unlock(&mutexColaDeListos);
	if (configuracion->algoritmoPlanificacion == ALGORITMO_SJF_CD) {
		planificacionNecesaria = true;
	}
	//si no habia ESIs participando en el sistema, o los que habia estaban
	//bloqueados, el planificador puede comenzar a trabajar de nuevo
	if (previamenteVacia && !esiEnEjecucion) {
		trabajar();
	}
}

void planificar() {
	//me hago una pasadita para volar los bloqueados
	list_remove_by_condition(colaDeListos, evaluarBloqueoDeEsi);
	if (esiEnEjecucion && esiEnEjecucion->bloqueado) {
		esiEnEjecucion = NULL;
	}

	if (colaDeListos->elements_count == 0) {
		return;
	}

	//estimo segun configuracion
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
		esiRemoverClaveTomada(esi, clave);
	}
	free(clave);
}

void liberarClavesDeEsi(t_esi *esi) {
	while (esi->clavesTomadas->elements_count > 0) {
		char *clave = list_get(esi->clavesTomadas, 0);
		liberarClave(clave);
	}
}

void liberarPrimerProcesoBloqueado(char *clave) {
	if (dictionary_has_key(diccionarioBloqueados, clave)) {
		t_list *bloqueados = dictionary_get(diccionarioBloqueados, clave);
		if (bloqueados->elements_count > 0) {
			t_esi *esi = list_remove(bloqueados, 0);
			esi->bloqueado = false;
			list_add(colaDeListos, esi);
		}
		if (bloqueados->elements_count == 0) {
			dictionary_remove_and_destroy(diccionarioBloqueados, clave, list_destroy);
		}
	}
}

t_esi *esiNew(int* socket) {
	t_esi *esi = malloc(sizeof(t_esi));
	esi->clavesTomadas = list_create();
	esi->estimacion = configuracion->estimacionInicial;
	esi->socket = socket;
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
	liberarClavesDeEsi(esiEnEjecucion);
	list_add(colaDeFinalizados, esiEnEjecucion);
	esiEnEjecucion = NULL;
	planificacionNecesaria = true;
}

void ejecutarComandosConsola() {
	while (bufferConsola->elements_count > 0) {
		t_instruccion_consola *instruccion = list_get(bufferConsola, 0);
		switch (instruccion->instruccion) {
		case INSTRUCCION_BLOQUEAR:
			bloquearEsiPorConsola(instruccion->primerParametro, instruccion->segundoParametro);
			break;
		case INSTRUCCION_DESBLOQUEAR:
			liberarPrimerProcesoBloqueado(instruccion->primerParametro);
			break;
		case INSTRUCCION_TERMINAR:
			abortarEsiPorId(instruccion->primerParametro);
			break;
		case INSTRUCCION_DEADLOCK:
			analizarDeadlock();
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
	t_esi *esi;
	if (esi->id == esiEnEjecucion->id) {
		liberarClavesDeEsi(esi);
		esiDestroyer(esiEnEjecucion);
		planificacionNecesaria = true;
		return;
	}
	if (esi = encontrarEsiPorId(colaDeListos, id)) {
		liberarClavesDeEsi(esi);
		list_remove_and_destroy_element(colaDeListos, getIndexDeEsi(colaDeListos, esi), esiDestroyer);
		return;
	}

	void abortarEsiEnDiccionarioBloqueados(char *clave, t_list *bloqueados) {
		if (esi = encontrarEsiPorId(bloqueados, id)) {
			liberarClavesDeEsi(esi);
			list_remove_and_destroy_element(bloqueados, getIndexDeEsi(bloqueados, esi), esiDestroyer);
		}
	}

	dictionary_iterator(diccionarioBloqueados, abortarEsiEnDiccionarioBloqueados);
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
		if (esi->id == iesi->id) {
			return i;
		}
	}
	return -1;
}

void analizarDeadlock() {
	//esta funcion hace el analisis del deadlock por una entrada en el diccionario de bloqueados
	//se usa con la funcion de iteracion de diccionario sobre el diccionario de bloqueados
	void analizarDiccionarioBloqueados(char *clave, t_list *bloqueados) {
		//esta lista se va guardando los implicados en el posible deadlock
		t_list * empernados = list_create();
		if (dictionary_has_key(diccionarioClavesTomadas, clave)) {
			t_esi *tomadorDeClave = dictionary_get(diccionarioClavesTomadas, clave);
			//el deadlock va a ser posible si el tomador de la clave esta bloqueado (y la clave esta tomada por un esi)
			if (tomadorDeClave && tomadorDeClave->bloqueado) {
				//si no lo tengo en la lista de empernados, es porque no llegue a una dependencia circular
				if (getIndexDeEsi(empernados, tomadorDeClave) < 0) {
					list_add(empernados, tomadorDeClave);
					char *claveQueLoTieneBloqueado;
					//nombre feo si los hay, pero descriptivo
					void buscarClaveQueLoTieneBloqueadoAlEsi(char *clave, t_list *bloqueados) {
						if (getIndexDeEsi(bloqueados, tomadorDeClave) >= 0) {
							claveQueLoTieneBloqueado = clave;
						}
					}
					dictionary_iterator(diccionarioBloqueados, buscarClaveQueLoTieneBloqueadoAlEsi);
					if (claveQueLoTieneBloqueado) {
					analizarDiccionarioBloqueados(claveQueLoTieneBloqueado, dictionary_get(diccionarioBloqueados, claveQueLoTieneBloqueado));
					}
				} else  {
					//llegue a una dependencia circular! todos los empernados son los participantes de esta barbarie
					log_error(logger, "\nDEADLOCK ENCONTRADO\n");
					log_info(logger, "\nLos ESIs implicados son los siguientes:");
					int i;
					for (i = 0; i < empernados->elements_count; i++) {
						t_esi *empernado = list_get(empernados, i);
						log_info(logger, "\n Id: %s con claves: ", empernado->id);
						int j;
						for (j = 0; j < empernado->clavesTomadas->elements_count; j++) {
							log_info(logger, "\n%s ", list_get(empernado->clavesTomadas, j));
						}
					}
				}
			}
		}
		list_destroy(empernados);
	}
	//con esta iteracion analizo todas las claves del diccionario de bloqueados
	dictionary_iterator(diccionarioBloqueados, analizarDiccionarioBloqueados);
}
