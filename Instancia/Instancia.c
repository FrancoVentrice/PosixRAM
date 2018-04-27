/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

int main(int argn, char *argv[]) {
	limpiarPantalla();
	centrarTexto("Instancia PosixRAM");
	centrarTexto("==================");
	retardoSegundos(1);
	logger = log_create("LogInstancia", "Instancia", true, LOG_LEVEL_INFO);
	log_info(logger,"Iniciando Instancia PosixRAM");

	cargarConfiguracion();
	retardoSegundos(5);
	finalizar(0);
}

/* termina el proceso correctamente liberando recursos */
void finalizar(int codigo) {
	log_info(logger,"Instancia %s finalizada" , configuracion->nombreDeInstancia);
	limpiarConfiguracion();
	log_destroy(logger);
	exit(codigo);
}

/* carga el archivo de configuracion default */
int cargarConfiguracion() {
	log_info(logger,"Cargando archivo de configuración");

	configuracion = malloc(sizeof(t_configuracion));

	//en eclipse cambia el path desde donde se corre, asi que probamos desde /Debug y desde /Instancia
	fd_configuracion = config_create("../Instancia.conf");
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create("Instancia.conf");
	}

	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.");
		return -1;
	}

	configuracion->ipCoordinador = config_get_string_value(fd_configuracion, "IP_COORDINADOR");
	configuracion->puertoCoordinador = config_get_int_value(fd_configuracion, "PUERTO_COORDINADOR");
	char *algoritmo = config_get_string_value(fd_configuracion, "ALGORITMO_REEMPLAZO");
	if (strcmp(algoritmo, "CIRC") == 0) {
		configuracion->algoritmoDeReemplazo = ALGORITMO_CIRC;
	} else if (strcmp(algoritmo, "LRU") == 0) {
		configuracion->algoritmoDeReemplazo = ALGORITMO_LRU;
	} else if (strcmp(algoritmo, "BSU") == 0) {
		configuracion->algoritmoDeReemplazo = ALGORITMO_BSU;
	}
	configuracion->puntoDeMontaje = config_get_string_value(fd_configuracion, "PUNTO_MONTAJE");
	configuracion->nombreDeInstancia = config_get_string_value(fd_configuracion, "NOMBRE_INSTANCIA");
	configuracion->intervaloDump = config_get_int_value(fd_configuracion, "INTERVALO_DUMP");

	log_info(logger,"Configuración cargada correctamente.");
	return 0;
}

/* libera los recursos de la configuracion */
void limpiarConfiguracion() {
	free(configuracion);
	config_destroy(fd_configuracion);
}

/* valida que la configuracion este completa (no valida errores) */
int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "IP_COORDINADOR")
		&& config_has_property(fd_configuracion, "PUERTO_COORDINADOR")
		&& config_has_property(fd_configuracion, "ALGORITMO_REEMPLAZO")
		&& config_has_property(fd_configuracion, "PUNTO_MONTAJE")
		&& config_has_property(fd_configuracion, "NOMBRE_INSTANCIA")
		&& config_has_property(fd_configuracion, "INTERVALO_DUMP"));
}
