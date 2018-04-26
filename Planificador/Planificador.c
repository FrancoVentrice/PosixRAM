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

	t_log *logPlanificador = log_create("planificador.log", "PLANIFICADOR",
	true, LOG_LEVEL_INFO);
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
	//char * respuesta = malloc(100);
	char encabezadoMensaje;
	tSolicitudESI *solicitud = malloc(sizeof(tSolicitudESI));
	solicitud->mensaje = malloc(100);
	tSolicitudESI* solicitudESI = malloc(sizeof(tSolicitudESI));
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
						&pkgHandshakeRespuesta, logPlanificador,
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
