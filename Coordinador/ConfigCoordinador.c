#include "Coordinador.h"

int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(fd_configuracion, "ALGORITMO_DISTRIBUCION")
		&& config_has_property(fd_configuracion, "CANTIDAD_ENTRADAS")
		&& config_has_property(fd_configuracion, "TAMANIO_ENTRADA")
		&& config_has_property(fd_configuracion, "RETARDO"));
}

int cargarConfiguracion() {
	logger = log_create("LogCoordinador", "Coordinador", true, LOG_LEVEL_INFO);
	logDeOperaciones = log_create("LogDeOperaciones", "Coordinador", true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(t_configuracion));
	diccionarioClaves = dictionary_create();
	instancias = list_create();
	punteroEL = 0;
	operacion = malloc(sizeof(t_operacionESI));

	//en eclipse cambia el path desde donde se corre, asi que probamos desde /Debug y desde /Coordinador
	fd_configuracion = config_create("../Coordinador.conf");
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create("Coordinador.conf");
	}

	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.","ERROR");
		return -1;
	}

	configuracion->puertoEscucha = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA");
	char *algoritmo = config_get_string_value(fd_configuracion, "ALGORITMO_DISTRIBUCION");
	if (strcmp(algoritmo, "LSU") == 0) {
		configuracion->algoritmoDistribucion = ALGORITMO_LSU;
	} else if (strcmp(algoritmo, "EL") == 0) {
		configuracion->algoritmoDistribucion = ALGORITMO_EL;
	} else if (strcmp(algoritmo, "KE") == 0) {
		configuracion->algoritmoDistribucion = ALGORITMO_KE;
	}
	configuracion->cantidadDeEntradas = config_get_int_value(fd_configuracion, "CANTIDAD_ENTRADAS");
	configuracion->tamanioDeEntrada = config_get_int_value(fd_configuracion, "TAMANIO_ENTRADA");
	configuracion->retardo = config_get_int_value(fd_configuracion, "RETARDO");

	log_info(logDeOperaciones, "\n\n-----------------Comienzo de sesion-----------------\n\n");
	log_info(logger,
		"\nPUERTO_ESCUCHA: %d\n"
		"ALGORITMO_DISTRIBUCION: %d\n"
		"CANTIDAD_ENTRADAS: %d\n"
		"TAMANIO_ENTRADA: %d\n"
		"RETARDO: %d\n" ,
		configuracion->puertoEscucha, configuracion->algoritmoDistribucion , configuracion->cantidadDeEntradas ,
		configuracion->tamanioDeEntrada, configuracion->retardo);
	return 0;
}

void limpiarConfiguracion() {
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);
	log_destroy(logDeOperaciones);
	dictionary_destroy_and_destroy_elements(diccionarioClaves, instanciaDestroyer);
	list_destroy_and_destroy_elements(instancias, instanciaDestroyer);
}
