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

	int puertoConexion = configuracion->puertoPlanificador;
	logger = log_create("cliente.log", "CLIENTE", 1, LOG_LEVEL_TRACE);
	int socket_servidor = connectToServer("127.0.0.1", puertoConexion, logger);
	finalizar(0);
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}
