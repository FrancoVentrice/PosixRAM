/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"
#include "../shared/pantalla.h"

void pantallaInicio() {
	limpiarPantalla();
	printf(CYAN_T);
	centrarTexto("Instancia PosixRAM para ReDistinto");
	centrarTexto("==================================");
	printf(RESET);
	fflush(stdout);
}

void mostrarConfiguracion() {
	if(parametrosEntrada.logPantalla)
		return;

	printf("\nArchivo de configuración cargado: " BOLD "%s\n" RESET, parametrosEntrada.archivoConf);
	printf(" - Instancia: " BOLD "%s\n" RESET, configuracion->nombreDeInstancia);
	printf(" - Algoritmo de reemplazo: " BOLD "%s\n" RESET, config_get_string_value(fd_configuracion, "ALGORITMO_REEMPLAZO"));
	printf(" - Punto de montaje: " BOLD "%s\n" RESET, configuracion->puntoDeMontaje);
	printf(" - Intervalo para dump: " BOLD "%d segundos\n" RESET, configuracion->intervaloDump);
	fflush(stdout);
}

void mostrarConexionCoordinador(void) {
	if(parametrosEntrada.logPantalla)
		return;

	printf("Conectado con Coordinador - IP: " BOLD "%s" RESET " - Puerto: " BOLD "%d\n" RESET ,
				configuracion->ipCoordinador,
				configuracion->puertoCoordinador);
	fflush(stdout);
}

void mostrarTablaDeEntradas(void) {
	if(parametrosEntrada.logPantalla)
		return;

	printf("Preparado espacio de almacenamiento para " BOLD "%d" RESET " entradas de " BOLD "%d bytes" RESET ". Entradas disponibles: " BOLD "%d" RESET ".\n"
				,configuracion->cantidadEntradas
				,configuracion->tamanioEntrada
				,entradasDisponibles());
	fflush(stdout);
}

void pantallaFin() {
	printf(AMARILLO_T);
	centrarTexto("PosixRAM (c) 2018");
	printf(RESET);
	fflush(stdout);
}

void mostrarMenu() {
	if(parametrosEntrada.logPantalla)
		return;

	printf("\n" BOLD "(C) " RESET "Forzar " UNDERLINE "C" RESET "ompactación - "
			BOLD "(D) " RESET "Forzar " UNDERLINE "D" RESET "ump - "
			BOLD "(E) " RESET "Listar " UNDERLINE "E" RESET "ntradas\n"
			BOLD "(L) " RESET "Últimas 10 líneas del " UNDERLINE "L" RESET "og - "
			BOLD "(R) " RESET UNDERLINE "R" RESET "efresh Status - "
			BOLD "(Q) " RESET UNDERLINE "Q" RESET "uit (salir)\n");
	fflush(stdout);
}

void mostrarTexto(char *cadena) {
	if(parametrosEntrada.logPantalla)
		return;

	puts(cadena);
	fflush(stdout);
}
