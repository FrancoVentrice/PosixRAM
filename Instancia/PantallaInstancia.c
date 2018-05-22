/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

void pantallaInicio() {
	limpiarPantalla();
	printf("\e[36m");
	centrarTexto("Instancia PosixRAM para ReDistinto");
	centrarTexto("==================================");
	printf("\e[0m");
}

void mostrarConfiguracion() {
	if(parametrosEntrada.logPantalla)
		return;

	printf("\nArchivo de configuración cargado:\033[1m\033[37m %s\033[0m", parametrosEntrada.archivoConf);
	printf("\n - Instancia:\033[1m\033[37m %s\033[0m", configuracion->nombreDeInstancia);
	printf("\n - Algoritmo de reemplazo:\033[1m\033[37m %s\033[0m", config_get_string_value(fd_configuracion, "ALGORITMO_REEMPLAZO"));
	printf("\n - Punto de montaje:\033[1m\033[37m %s\033[0m", configuracion->puntoDeMontaje);
	printf("\n - Intervalo para dump:\033[1m\033[37m %d segundos\033[0m\n", configuracion->intervaloDump);
}

void pantallaFin() {
	printf("\e[33m\n");
	centrarTexto("PosixRAM (c) 2018");
	printf("\e[0m\n");
}

void mostrarTexto(char *cadena) {
	if(parametrosEntrada.logPantalla)
		return;

	puts(cadena);
}
