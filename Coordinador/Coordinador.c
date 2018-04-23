/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Coordinador.h"
#include "..//shared/protocolo.h"

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	escucharConexiones();
}
void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}

void escucharConexiones() {

		int puertoEscucha = 8001;

		t_log *logPlanificador = log_create("coordinador.log", "COORDINADOR",
		true, LOG_LEVEL_INFO);
		int maxSock;
		int iSocketEscucha;
		int iSocketComunicacion;

		fd_set setSocketsOrquestador;
		FD_ZERO(&setSocketsOrquestador);

		// Inicializacion de sockets y actualizacion del log
		iSocketEscucha = crearSocketEscucha(puertoEscucha, logPlanificador);

		FD_SET(iSocketEscucha, &setSocketsOrquestador);
		maxSock = iSocketEscucha;

		tPaquete pkgHandshake;

		tMensaje *tipoMensaje = malloc(sizeof(tMensaje));
		char * sPayloadRespuesta = malloc(100);
		char * respuesta = malloc(100);

		char encabezadoMensaje;
		tSolicitudESI *solicitud = malloc(sizeof(tSolicitudESI));
		solicitud->mensaje = malloc(100);

		tSolicitudESI* solicitudESI = malloc(sizeof(tSolicitudESI));
		int recibidos;

		while (1) {
			puts("Escuchando");
			iSocketComunicacion = getConnection(&setSocketsOrquestador,
					&maxSock, iSocketEscucha, tipoMensaje, &sPayloadRespuesta,
					logPlanificador);

			printf("Socket comunicacion: %d \n", iSocketComunicacion);

			if (iSocketComunicacion != -1) {

				switch (*tipoMensaje) {

				case E_HANDSHAKE:

					puts("HANDSHAKE CON ESI");
					tSolicitudESI *mensaje = malloc(100);
					char* encabezado = malloc(10);

					deserializar(sPayloadRespuesta, "%c%s", encabezado,
							mensaje);
					strcpy(solicitud->mensaje, mensaje);
					printf("MENSAJE DE ESI: %s\n", solicitud->mensaje);
					break;
				case P_HANDSHAKE:
					puts("HANDSHAKE CON PLANIFICADOR");
					break;

					//conexion con el Planificador

				}
			}
		}
		finalizar(0);
	}


