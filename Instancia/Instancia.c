/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	finalizar(0);
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}
