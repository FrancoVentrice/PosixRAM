#include "Planificador.h"

int main() {
	cargarConfiguracion();
	consola();
	finalizar(0);
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}
