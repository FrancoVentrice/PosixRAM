/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "ESI.h"
#include "..//shared/protocolo.h"
#include "..//shared/sockets.c"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

int main(int argn, char *argv[]) {
	cargarConfiguracion();

	int bytesEnviados;
	int socketServidor = connectToServer(configuracion->ipPlanificador, configuracion->puertoPlanificador, logger);

	tSolicitudESI* solicitud = malloc(sizeof(tSolicitudESI));
	solicitud->mensaje=malloc(100);
	strcpy(solicitud->mensaje,"HOLA SOY ESI!!!");
	tPaquete pkgHandshake;
	pkgHandshake.type = E_HANDSHAKE;

	pkgHandshake.length = serializar(pkgHandshake.payload, "%c%s",
			pkgHandshake.type, solicitud->mensaje);

	puts("Se envia path al FS");
	bytesEnviados = enviarPaquete(socketServidor, &pkgHandshake,
			logger, "Se envia solicitud de ejecucion");
	printf("Se envian %d bytes\n", bytesEnviados);
	finalizar(0);
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}
