/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Planificador.h"
#include "..//shared/protocolo.h"

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	int puerto_escucha = 8000;
	fd_set master;
	fd_set read_fds;
	int *fdmax;
	int bytesEnviados;
	t_log *logPlanificador = log_create("planificador.log", "PLANIFICADOR",
	true, LOG_LEVEL_INFO);
	int maxSock;
	int iSocketEscucha;
	int iSocketComunicacion;
	int tamanioMensaje = 0;

	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log
	iSocketEscucha = crearSocketEscucha(8000, logPlanificador);

	FD_SET(iSocketEscucha, &setSocketsOrquestador);
	maxSock = iSocketEscucha;

	tPaquete pkgHandshake;

	tMensaje tipoMensaje;
	char * sPayloadRespuesta = malloc(100);
	char * respuesta = malloc(100);

	char encabezadoMensaje;
	tSolicitudESI *solicitud = malloc(sizeof(tSolicitudESI));
	solicitud->mensaje = malloc(100);

	tSolicitudESI* solicitudESI = malloc(sizeof(tSolicitudESI));
	int recibidos;

	while (1) {
		puts("Escuchando");
		iSocketComunicacion = getConnection(&setSocketsOrquestador, &maxSock,
				iSocketEscucha, &tipoMensaje, &sPayloadRespuesta,
				logPlanificador);

		printf("Socket comunicacion: %d \n", iSocketComunicacion);

		if (iSocketComunicacion != -1) {

			switch (tipoMensaje) {


						case E_HANDSHAKE:

							puts("HANDSHAKE CON ESI");
							tSolicitudESI *mensaje= malloc(100);
							char* encabezado=malloc(10);
							deserializar(sPayloadRespuesta, "%c%s",encabezado,mensaje);
									solicitud->mensaje=mensaje;
									printf("MENSAJE DE ESI: %s\n",mensaje);

							break;

							break;
						}
					}
				}
				consola();
				finalizar(0);
			}

			void finalizar(int codigo) {
				limpiarConfiguracion();
				exit(codigo);
			}
