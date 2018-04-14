#include"Coordinador.h"

int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(fd_configuracion, "ALGORITMO_DISTRIBUCION")
		&& config_has_property(fd_configuracion, "CANTIDAD_ENTRADAS")
		&& config_has_property(fd_configuracion, "TAMANIO_ENTRADA")
		&& config_has_property(fd_configuracion, "RETARDO"));
}

int cargarConfiguracion() {
	logger = log_create("LogCoordinador", "Coordinador", true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(t_configuracion));

	//en eclipse cambia el path desde donde se corre, asi que probamos desde /Debug y desde /Coordinador
	fd_configuracion = config_create("../config");
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create("config");
	}

	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.","ERROR");
		return -1;
	}

	configuracion->puerto = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA");
	configuracion->algoritmoDistribucion = config_get_string_value(fd_configuracion, "ALGORITMO_DISTRIBUCION");
	configuracion->cantidadDeEntradas = config_get_int_value(fd_configuracion, "CANTIDAD_ENTRADAS");
	configuracion->tamanioDeEntrada = config_get_int_value(fd_configuracion, "TAMANIO_ENTRADA");
	configuracion->retardo = config_get_int_value(fd_configuracion, "RETARDO");

	log_info(logger,
		"\nPUERTO_ESCUCHA: %d\n"
		"ALGORITMO_DISTRIBUCION: %s\n"
		"CANTIDAD_ENTRADAS: %d\n"
		"TAMANIO_ENTRADA: %d\n"
		"RETARDO: %d\n" ,
		configuracion->puerto, configuracion->algoritmoDistribucion , configuracion->cantidadDeEntradas ,
		configuracion->tamanioDeEntrada, configuracion->retardo);
	return 0;
}

void limpiarConfiguracion() {
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);
}
