/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Planificador.h"

void atenderESI(int* iSocketComunicacion) {
	//tArgs* my_data=malloc(sizeof(tArgs));
	int bytesEnviados;

	//my_data = (tArgs*) argumentos;
	log_info(logger, "HANDSHAKE CON ESI");
	t_esi* esiNuevo = esiNew(iSocketComunicacion);

	//lo agrego a listos y le aviso Ok de Handshake

	tRespuesta* respuestaEjecucion = malloc(sizeof(tRespuesta));
	respuestaEjecucion->mensaje = malloc(100);
	strcpy(respuestaEjecucion->mensaje, "OK HANDSHAKE");
	tPaquete pkgHandshakeRespuesta;
	pkgHandshakeRespuesta.type = P_HANDSHAKE;

	pkgHandshakeRespuesta.length = serializar(pkgHandshakeRespuesta.payload,
			"%c%s", pkgHandshakeRespuesta.type, respuestaEjecucion->mensaje);

	log_info(logger, "Tipo: %d, largo: %d \n", pkgHandshakeRespuesta.type,
			pkgHandshakeRespuesta.length);

	log_info(logger, "Se envia respuesta");
	bytesEnviados = enviarPaquete(*(esiNuevo->socket), &pkgHandshakeRespuesta,
			logger, "Se envia respuesta a ESI");

	log_info(logger, "Se envian %d bytes\n", bytesEnviados);

	agregarEsiAColaDeListos(esiNuevo);
}

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	escucharESIs();
	levantarConsola();
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
//se llama cuando una sentencia del ESI fue ejecutada correctamente (mas alla del resultado)
//se llama cuando la cola de listos vacia se popula
void trabajar() {
	if (ejecutando) {
		ejecutarComandosConsola();
		if (planificacionNecesaria) {
			planificar();
		}
		enviarOrdenDeEjecucion();
	}
}

void enviarOrdenDeEjecucion() {
	tRespuesta* autorizarEjecucion = malloc(sizeof(tRespuesta));
	consultaCoordinador=malloc(sizeof(tConsultaCoordinador));
	consultaCoordinador->clave=malloc(100);
	autorizarEjecucion->mensaje = malloc(100);
	strcpy(autorizarEjecucion->mensaje, "OK PARA EJECUTAR LINEA");
	tPaquete pkgHandshakeRespuesta;
	pkgHandshakeRespuesta.type = P_HANDSHAKE;

	pkgHandshakeRespuesta.length = serializar(pkgHandshakeRespuesta.payload,
			"%c%s", pkgHandshakeRespuesta.type,
			autorizarEjecucion->mensaje);

	log_info(logger, "Tipo: %d, largo: %d \n", pkgHandshakeRespuesta.type,
			pkgHandshakeRespuesta.length);

	log_info(logger, "Se envia orden para ejecutar");
	int bytesEnviados = enviarPaquete(*(esiEnEjecucion->socket),
			&pkgHandshakeRespuesta, logger, "Se envia orden para ejecutar");

	log_info(logger, "Se envian %d bytes\n", bytesEnviados);

	//recibir respuesta de esi si la linea es correcta:
	//
	tRespuesta *respuestaESI = malloc(sizeof(tRespuesta));

	tMensaje tipoMensajeEsi;
	char * sPayloadRespuestaHand = malloc(100);

	int bytesRecibidos = recibirPaquete(*(esiEnEjecucion->socket), &tipoMensajeEsi,
			&sPayloadRespuestaHand, logger, "Hand Respuesta");
	log_info(logger, "RECIBIDOS:%d", bytesRecibidos);
	respuestaESI->mensaje = malloc(10);

	deserializar(sPayloadRespuestaHand, "%s",respuestaESI->mensaje);
	log_info(logger, "Respuesta Linea: %s", respuestaESI->mensaje);


	if (strcmp(respuestaESI->mensaje,"OK")==0) {

		//ESPERO MENSAJE DEL COORDINADOR
	recibirConsultaOperacion(consultaCoordinador);

	evaluarConsultaDeOperacion();

	}else esiFinalizado();


	// -OK (el esi va a mandar una linea al coordinador)
	//  Planificador no hace nada y queda a la espera de la respuesta del coordinador
	// 	ejecuta un metodo que espere la consulta del coordinador sobre la clave a usar:
	//
	 //este metodo envia un par de mensajes al coordinador
	//
	//} else if (esi finalizado) {
	// -Esi finalizado (trató de leer linea y se encontro con final de archivo)
	//    replanifica y asigna a otro esi

	// OJO ACA QUE VUELVE A QUERER ENVIAR UN ESI QUE ESTA EN NULL
	//}					//LLAMA A 'TRABAJAR' Y VUELVE A ESTE METODO CON
						// UN ESI EN NULL

}
void recibirConsultaOperacion(){
	tMensaje tipoMensaje;
	char * sPayloadConsulta = malloc(100);


	int bytesRecibidos = recibirPaquete(socketCoordinador, &tipoMensaje,
			&sPayloadConsulta, logger, "Consulta coordinador");
	log_info(logger, "RECIBIDOS:%d", bytesRecibidos);

	switch(tipoMensaje){

	case C_CONSULTA_OPERACION_GET:
		deserializar(sPayloadConsulta, "%s", consultaCoordinador->clave);
		log_info(logger,"Operacion GET para clave %s: ",consultaCoordinador->clave);
		consultaCoordinador->operacion=1;
		break;


	case C_CONSULTA_OPERACION_SET:
		deserializar(sPayloadConsulta, "%s%s", consultaCoordinador->clave,
				consultaCoordinador->valor);
		log_info(logger,"Operacion GET para clave %s y valor %s: ",consultaCoordinador->clave,
				consultaCoordinador->valor);

		consultaCoordinador->operacion=2;

		break;

	case C_CONSULTA_OPERACION_STORE:
		deserializar(sPayloadConsulta, "%s", consultaCoordinador->clave);
		consultaCoordinador->operacion=3;
		log_info(logger,"Operacion STORE para clave %s: ",consultaCoordinador->clave);

		break;

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
	nId=0;


	int puertoEscucha = configuracion->puerto;
	int puertoConexion = configuracion->puertoCoordinador;
	char* ipCoordinador = configuracion->ipCoordinador;

	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	//Conexion al Coordinador
	socketCoordinador = connectToServer(ipCoordinador, puertoConexion,
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

	log_info(logger,"Se envia solicitud de ejecucion");
	tamanioTotal = enviarPaquete(socketCoordinador, &pkgHandshakeCoordinador,
			logger, "Se envia solicitud de ejecucion");
	log_info("Se envian %d bytes\n", tamanioTotal);

	//RESPUESTA DEL COORDINADOR
	tMensaje tipoMensaje;
	char * sPayloadRespuestaHand = malloc(100);

	int bytesRecibidos = recibirPaquete(socketCoordinador, &tipoMensaje,
			&sPayloadRespuestaHand, logger, "Hand Respuesta");
	log_info(logger,"RECIBIDOS:%d", bytesRecibidos);
	respuesta->mensaje = malloc(10);
	char encabezado_mensaje;

	deserializar(sPayloadRespuestaHand, "%c%s", &encabezado_mensaje,
			respuesta->mensaje);
	log_info(logger,"RESPUESTA: %s", respuesta->mensaje);

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
	log_info(logger,"Escuchando");

	while (1) {
		iSocketComunicacion = getConnection(&setSocketsOrquestador, &maxSock,
				iSocketEscucha, &tipoMensaje, &sPayloadRespuesta, logger);

		if (iSocketComunicacion != -1) {
			switch (tipoMensaje) {
			case E_HANDSHAKE:
/*
				//my_data = (tArgs*) argumentos;
				log_info(logger, "HANDSHAKE CON ESI");
				t_esi* esiNuevo = esiNew(&iSocketComunicacion);

				//lo agrego a listos y le aviso Ok de Handshake
				//agregarEsiAColaDeListos(esiNuevo);

				tRespuesta* respuestaEjecucion = malloc(sizeof(tRespuesta));
				respuestaEjecucion->mensaje = malloc(100);
				strcpy(respuestaEjecucion->mensaje, "OK HANDSHAKE");
				tPaquete pkgHandshakeRespuesta;
				pkgHandshakeRespuesta.type = P_HANDSHAKE;

				pkgHandshakeRespuesta.length = serializar(
						pkgHandshakeRespuesta.payload, "%c%s",
						pkgHandshakeRespuesta.type,
						respuestaEjecucion->mensaje);

				log_info(logger, "Tipo: %d, largo: %d ",
						pkgHandshakeRespuesta.type,
						pkgHandshakeRespuesta.length);

				log_info(logger, "Se envia respuesta");
				bytesEnviados = enviarPaquete(*(esiNuevo->socket),
						&pkgHandshakeRespuesta, logger,
						"Se envia respuesta a ESI");

				log_info(logger, "Se envian %d bytes", bytesEnviados);*/
				//creo el hilo para el ESI que quiero atender
				//(&hiloESI,NULL,atenderESI,&iSocketComunicacion);
			//	pthread_join(hiloESI,NULL);
				atenderESI(&iSocketComunicacion);
				//le envio el OK a ESI para ejecutar
				tipoMensaje = DESCONEXION;
				break;

			case DESCONEXION:
				break;
			}
		}
	}
}

void recibirResultadoOperacion() {
	tRespuestaCoordinador* respuestaCoordinador = malloc(sizeof(tRespuestaCoordinador));
	tMensaje tipoMensajeCoordinador;
	int recibidos;
	char * sPayloadRespuestaCoordinador = malloc(100);

	log_info(logger, "Esperando respuesta de la operacion...");
	recibidos = recibirPaquete(socketCoordinador,
			&tipoMensajeCoordinador, &sPayloadRespuestaCoordinador, logger,
			"Respuesta a la ejecucion");
	log_info(logger, "RECIBIDOS:%d", recibidos);
	respuestaCoordinador->mensaje = malloc(100);

	deserializar(sPayloadRespuestaCoordinador, "%s",
			respuestaCoordinador->mensaje);

	log_info(logger, "RESPUESTA OPERACION DEL COORDINADOR : %s",
			respuestaCoordinador->mensaje);

	//if (OK) {
	trabajar();
	//}
}

void enviarOperacionValida() {
	//aca se envia al coordinador que la operacion sobre la clave es valida
	tPaquete pkgOperacionValida;
	int enviados;
	char* respuesta=malloc(10);
	strcpy(respuesta,"OK");
	pkgOperacionValida.type=P_RESPUESTA_CONSULTA;

	pkgOperacionValida.length = serializar(pkgOperacionValida.payload,
			"%s",respuesta);

		log_info(logger,"Se envia respuesta consulta");
		enviados = enviarPaquete(socketCoordinador, &pkgOperacionValida,
				logger, "Se envia respuesta consulta");
		log_info("Se envian %d bytes\n", enviados);
	//
	//
	//para finalmente esperar el resultado de la operacion
	//recibirResultadoOperacion();
}

void evaluarConsultaDeOperacion() {
	char *clave=consultaCoordinador->clave;
	int operacion=consultaCoordinador->operacion;
	switch (operacion) {
	case 1: //GET: numeros puestos a modo de prueba
		if (evaluarBloqueoDeClave(clave)) {
			t_esi *esi = dictionary_get(diccionarioClavesTomadas, clave);
			if (strcmp(esiEnEjecucion->id, esi->id) == 0) {
				enviarOperacionValida();
				esiTomaClave(esiEnEjecucion, clave);
			} else {
				//informar al coordinador que la clave ya esta tomada
				bloquearESIConClave(esiEnEjecucion, clave);
				planificacionNecesaria = true;
				trabajar();
			}
		} else {
			enviarOperacionValida();
			esiTomaClave(esiEnEjecucion, clave);
		}
		break;
	case 2: //SET: numeros puestos a modo de prueba
		if (evaluarBloqueoDeClave(clave)) {
			t_esi *esi = dictionary_get(diccionarioClavesTomadas, clave);
			if (strcmp(esiEnEjecucion->id, esi->id) == 0) {
				enviarOperacionValida();
			} else {
				//informar al coordinador que la clave ya esta tomada
				bloquearESIConClave(esiEnEjecucion, clave);
				planificacionNecesaria = true;
				trabajar();
			}
		} else {
			//informar al coordinador que la clave esta libre pero el esi no la tiene asignada
			//creo que es un error
		}
		break;
	case 3: //STORE: numeros puestos a modo de prueba
		if (evaluarBloqueoDeClave(clave)) {
			t_esi *esi = dictionary_get(diccionarioClavesTomadas, clave);
			if (strcmp(esiEnEjecucion->id, esi->id) == 0) {
				enviarOperacionValida();
				liberarClave(clave);
			} else {
				//informar al coordinador que la clave ya esta tomada
				bloquearESIConClave(esiEnEjecucion, clave);
				planificacionNecesaria = true;
				trabajar();
			}
		} else {
			//informar al coordinador que la clave esta libre pero el esi no la tiene asignada
			//creo que es un error
		}
		break;
	}
}

void agregarEsiAColaDeListos(t_esi *esi) {
	pthread_mutex_lock(&mutexColaDeListos);
	bool previamenteVacia = colaDeListos->elements_count == 0;
	list_add(colaDeListos, esi);
	pthread_mutex_unlock(&mutexColaDeListos);
	if (configuracion->algoritmoPlanificacion == ALGORITMO_SJF_CD) {
		planificacionNecesaria = true;
	} else if (configuracion->algoritmoPlanificacion == ALGORITMO_HRRN) {
		esi->instanteLlegadaAListos = tiempoTotalEjecucion;
	}
	//si no habia ESIs participando en el sistema, o los que habia estaban
	//bloqueados, el planificador puede comenzar a trabajar de nuevo
	if (previamenteVacia && !esiEnEjecucion) {
		esiEnEjecucion=esi;
		planificacionNecesaria = true;
		trabajar();
	}
	//Si entra otro ESI entonces se queda en cola de listos esperando el ok del planificador
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
		return;
	}

	//si ya habia uno en ejecucion, lo devuelvo a la cola de listos
	if (esiEnEjecucion) {
		list_add(colaDeListos, esiEnEjecucion);
	}

	esiEnEjecucion = list_get(colaDeListos, indexProximoAEjecutar);
	list_remove(colaDeListos, indexProximoAEjecutar);
}

void sentenciaEjecutadaCorrectamente() {
	esiEnEjecucion->rafagaAnterior ++;
	esiEnEjecucion->estimacion--;
	tiempoTotalEjecucion ++;
}

void esiFinalizado() {
	finalizarEsiEnEjecucion();
	planificacionNecesaria = true;
	trabajar();
}

int planificarHRRN() {
	t_esi* esiHRRNMayor;
	float RRMayor = 0;
	int indexHRRNMayor = 0;

	int i;
	for (i = 0; i < colaDeListos->elements_count; i++) {
		t_esi* esiActual = list_get(colaDeListos, i);
		int tiempoEspera = calcularTiempoEspera(esiActual);
		float responseRatio = ((float) tiempoEspera
				+ (float) esiActual->estimacion)
							/ (float) esiActual->estimacion;

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
			estimar(esi);
			if (configuracion->algoritmoPlanificacion == ALGORITMO_HRRN) {
				esi->instanteLlegadaAListos = tiempoTotalEjecucion;
			}
			list_add(colaDeListos, esi);
		}
		if (bloqueados->elements_count == 0) {
			dictionary_remove_and_destroy(diccionarioBloqueados, clave, list_destroy);
		}
	}
}

t_esi *esiNew(int* socket) {
	nId++;
	t_esi *esi = malloc(sizeof(t_esi));
	esi->id=string_new();
	string_append_with_format(&(esi->id),"%s %d","ESI",nId);
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
		if (strcmp(esi->id, iesi->id) == 0) {
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
