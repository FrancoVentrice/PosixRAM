#include "Coordinador.h"

int main() {
	cargarConfiguracion();
	finalizar(0);
}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}
