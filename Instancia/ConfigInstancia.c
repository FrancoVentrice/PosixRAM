/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

int cargarConfiguracion() {
	/* carga el archivo de configuracion default */

	log_info(logger,"Cargando archivo de configuración: %s", parametrosEntrada.archivoConf);

	fd_configuracion = config_create(parametrosEntrada.archivoConf);
	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.");
		mostrarTexto("Error al cargar el archivo de configuración.");
		return 0;
	}

	configuracion.nombreDeInstancia = config_get_string_value(fd_configuracion, "NOMBRE_INSTANCIA");

	configuracion.ipCoordinador = config_get_string_value(fd_configuracion, "IP_COORDINADOR");
	configuracion.puertoCoordinador = config_get_int_value(fd_configuracion, "PUERTO_COORDINADOR");
	configuracion.fdSocketCoordinador = -1;

	char *algoritmo = config_get_string_value(fd_configuracion, "ALGORITMO_REEMPLAZO");
	// lo que sigue es necesario porque C no tiene un switch de strings ::facepalm::
	if (strcmp(algoritmo, "CIRC") == 0) {
		configuracion.algoritmoDeReemplazo = ALGORITMO_CIRC;
	} else if (strcmp(algoritmo, "LRU") == 0) {
		configuracion.algoritmoDeReemplazo = ALGORITMO_LRU;
	} else if (strcmp(algoritmo, "BSU") == 0) {
		configuracion.algoritmoDeReemplazo = ALGORITMO_BSU;
	}

	configuracion.puntoDeMontaje = config_get_string_value(fd_configuracion, "PUNTO_MONTAJE");
	configuracion.intervaloDump.it_value.tv_sec = config_get_int_value(fd_configuracion, "INTERVALO_DUMP");
	configuracion.intervaloDump.it_value.tv_nsec = 0;
	configuracion.intervaloDump.it_interval.tv_sec = config_get_int_value(fd_configuracion, "INTERVALO_DUMP");
	configuracion.intervaloDump.it_interval.tv_nsec = 0;
	configuracion.fdTimerDump = -1;

	log_info(logger,"Configuración cargada correctamente.");
	log_info(logger," - Instancia: %s", configuracion.nombreDeInstancia);
	log_info(logger," - Algoritmo de reemplazo: %s", algoritmo);
	log_info(logger," - Punto de montaje: %s", configuracion.puntoDeMontaje);
	log_info(logger," - Intervalo para dump: %d segundos", (int)configuracion.intervaloDump.it_value.tv_sec);
	return 1;
}

int configValida(t_config* fd_configuracion) {
	/* valida que la configuracion este completa (no valida errores) */

	return (config_has_property(fd_configuracion, "NOMBRE_INSTANCIA")
		&& config_has_property(fd_configuracion, "IP_COORDINADOR")
		&& config_has_property(fd_configuracion, "PUERTO_COORDINADOR")
		&& config_has_property(fd_configuracion, "ALGORITMO_REEMPLAZO")
		&& config_has_property(fd_configuracion, "PUNTO_MONTAJE")
		&& config_has_property(fd_configuracion, "INTERVALO_DUMP"));
}

void limpiarConfiguraion() {
	/* libera los recursos asignados para la configuración */
	if (fd_configuracion != NULL) {
		config_destroy(fd_configuracion);
	}
}
