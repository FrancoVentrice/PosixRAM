/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Planificador.h"

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

	int puertoEscucha = configuracion->puerto;
	int puertoConexion = configuracion->puertoCoordinador;
	char* ipCoordinador=configuracion->ipCoordinador;

	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	//Conexion al Coordinador
	int socketCoordinador = connectToServer("127.0.0.1", puertoConexion,
			logger);
	tSolicitudPlanificador* solicitudPlanificador = malloc(
			sizeof(tSolicitudESI));
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
				printf("Socket comunicacion: %d \n", iSocketComunicacion);

				puts("HANDSHAKE CON ESI");
				tSolicitudESI *mensaje = malloc(100);
				char* encabezado = malloc(10);
				deserializar(sPayloadRespuesta, "%c%s", encabezado, respuesta->mensaje);
				printf("MENSAJE DE ESI: %s\n", respuesta->mensaje);

				//le envio el OK a ESI para ejecutar

				tRespuesta* respuesta = malloc(sizeof(tRespuesta));
				respuesta->mensaje = malloc(100);
				strcpy(respuesta->mensaje, "OK");
				tPaquete pkgHandshakeRespuesta;
				pkgHandshakeRespuesta.type = P_HANDSHAKE;

				pkgHandshakeRespuesta.length = serializar(
						pkgHandshakeRespuesta.payload, "%c%s",
						pkgHandshakeRespuesta.type, respuesta->mensaje);

				printf("Tipo: %d, largo: %d \n", pkgHandshakeRespuesta.type,
						pkgHandshakeRespuesta.length);

				puts("Se envia respuesta");
				bytesEnviados = enviarPaquete(iSocketComunicacion,
						&pkgHandshakeRespuesta, logger,
						"Se envia respuesta a ESI");
				printf("Se envian %d bytes\n", bytesEnviados);
				tipoMensaje=DESCONEXION;

				break;

			case DESCONEXION:
				break;
			}
		}
	}
}

//se corre cuando hay que elegir un nuevo ESI a ejecutar
//si es con desalojo, tambien se corre cuando entra un nuevo proceso a la lista
//setea la estimacion de todos los ESIs en la cola de listos
//agrega el ESI en ejecucion a la cola de listos para hacer bien los calculos, asi que no hace falta agregarlo antes
//setea el proximo ESI a ejecutar
void estimarSJF() {
	list_add(colaDeListos, esiEnEjecucion);
	int i;
	t_esi *ESIMasCorto = list_get(colaDeListos, 0);
	int indexDelESIMasCorto;
	float alfa = configuracion->alfa / 100;

	//itera hasta "elements_count - 1" porque no quiero que reestime al ESI en
	//ejecucion, el cual agregue al final de la lista
	for (i = 0; i < colaDeListos->elements_count - 1; i++) {
		t_esi *esi = list_get(colaDeListos, i);
		//rafagaAnterior lo uso como flag para saber si la estimacion esta actualizada
		if (esi->rafagaAnterior != 0) {
		esi->estimacionAnterior = esi->estimacion;
		esi->estimacion = alfa * esi->rafagaAnterior + (1 - alfa) * esi->estimacionAnterior;
		//"actualizo" la estimacion seteando en 0 la rafaga anterior, total ya no la voy a usar mas
		esi->rafagaAnterior = 0;
		}
		if (esi->estimacion < ESIMasCorto->estimacion) {
			ESIMasCorto = esi;
			indexDelESIMasCorto = i;
		}
	}

	//si lo habia bloqueado, me voy olvidando de dejarlo en la lista de listos
	if (esiEnEjecucion->bloqueado) {
		list_remove(colaDeListos, colaDeListos->elements_count - 1);
	}

	//finalmente, comparo al mas corto de la cola de listos con el que ya estaba ejecutando
	//la prioridad en caso de igualdad la tiene el que ya estaba ejecutando
	//si estaba bloqueado, obviamente lo reemplazo directamente
	if (esiEnEjecucion->bloqueado || ESIMasCorto->estimacion < esiEnEjecucion->estimacion) {
		esiEnEjecucion = ESIMasCorto;
		list_remove(colaDeListos, indexDelESIMasCorto);
	} else {
		//remuevo de listos al que ya estaba ejecutando (que lo habia agregado al final de la lista), vuelve a seguir ejecutando
		//si el esi ejecutando no se habia bloqueado, no pasa por el otro if que lo remueve, asi que no se genera inconsistencia
		list_remove(colaDeListos, colaDeListos->elements_count - 1);
	}
}

void sentenciaEjecutadaCorrectamenteSJF() {
	esiEnEjecucion->rafagaAnterior ++;
}

void bloquearESIConClave(t_esi *esi, char *clave) {
	esi->bloqueado = true;
	if (dictionary_has_key(diccionarioBloqueados, clave)) {
		t_list *bloqueados = dictionary_get(diccionarioBloqueados, clave);
		list_add(bloqueados, esi);
	} else {
		t_list *nuevaCola = list_create();
		list_add(nuevaCola, esi);
		dictionary_put(diccionarioBloqueados, clave, nuevaCola);
	}
}

void bloquearClaveSola(char *clave) {
	if (!dictionary_has_key(diccionarioBloqueados, clave)) {
		t_list *nuevaCola = list_create();
		dictionary_put(diccionarioBloqueados, clave, nuevaCola);
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
			list_remove(bloqueados, i);
		}
		dictionary_remove(diccionarioBloqueados, clave);
		free(bloqueados);
	}
}
