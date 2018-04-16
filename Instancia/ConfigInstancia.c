#include"Instancia.h"

int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "IP_COORDINADOR")
		&& config_has_property(fd_configuracion, "PUERTO_COORDINADOR")
		&& config_has_property(fd_configuracion, "ALGORITMO_REEMPLAZO")
		&& config_has_property(fd_configuracion, "PUNTO_MONTAJE")
		&& config_has_property(fd_configuracion, "NOMBRE_INSTANCIA")
		&& config_has_property(fd_configuracion, "INTERVALO_DUMP"));
}

int cargarConfiguracion() {
	logger = log_create("LogInstancia", "Instancia", true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(t_configuracion));

	//en eclipse cambia el path desde donde se corre, asi que probamos desde /Debug y desde /Instancia
	fd_configuracion = config_create("../config");
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create("config");
	}

	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.","ERROR");
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

	log_info(logger,
		"\nIP_COORDINADOR: %s\n"
		"PUERTO_COORDINADOR: %d\n"
		"ALGORITMO_REEMPLAZO: %d\n"
		"PUNTO_MONTAJE: %s\n"
		"NOMBRE_INSTANCIA: %s\n"
		"INTERVALO_DUMP: %d\n",
		configuracion->ipCoordinador, configuracion->puertoCoordinador , configuracion->algoritmoDeReemplazo ,
		configuracion->puntoDeMontaje, configuracion->nombreDeInstancia, configuracion->intervaloDump);
	return 0;
}

void limpiarConfiguracion() {
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);
}
