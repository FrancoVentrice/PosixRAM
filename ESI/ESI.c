/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "ESI.h"

int main() {
	cargarConfiguracion();
	finalizar(0);
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}
