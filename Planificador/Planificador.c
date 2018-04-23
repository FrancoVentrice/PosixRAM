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

	int puertoEscucha = 8000;
	int puertoConexion = 8001;
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

	// Inicializacion de sockets y actualizacion del log
	iSocketEscucha = crearSocketEscucha(configuracion->puerto, logger);

	FD_SET(iSocketEscucha, &setSocketsOrquestador);
	maxSock = iSocketEscucha;

	tPaquete pkgHandshake;
	tMensaje tipoMensaje;
	char * sPayloadRespuesta = malloc(100);
	//char * respuesta = malloc(100);
	char encabezadoMensaje;
	tSolicitudESI *solicitud = malloc(sizeof(tSolicitudESI));
	solicitud->mensaje = malloc(100);
	tSolicitudESI* solicitudESI = malloc(sizeof(tSolicitudESI));
	int recibidos;

	while (1) {
		puts("Escuchando");
		iSocketComunicacion = getConnection(&setSocketsOrquestador, &maxSock,
				iSocketEscucha, &tipoMensaje, &sPayloadRespuesta, logger);
		printf("Socket comunicacion: %d \n", iSocketComunicacion);

		if (iSocketComunicacion != -1) {
			switch (tipoMensaje) {
			case E_HANDSHAKE:

				puts("HANDSHAKE CON ESI");
				tSolicitudESI *mensaje = malloc(100);
				char* encabezado = malloc(10);
				deserializar(sPayloadRespuesta, "%c%s", encabezado, mensaje);
				strcpy(solicitud->mensaje, mensaje);
				printf("MENSAJE DE ESI: %s\n", solicitud->mensaje);

				//le envio el OK a ESI para ejecutar

				tRespuesta* respuesta = malloc(sizeof(tRespuesta));
				respuesta->mensaje = malloc(10);
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

				break;
			}
		}
	}
}
