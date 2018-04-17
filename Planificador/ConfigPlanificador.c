#include"Planificador.h"

int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(fd_configuracion, "ALGORITMO_PLANIFICACION")
		&& config_has_property(fd_configuracion, "ESTIMACION_INICIAL")
		&& config_has_property(fd_configuracion, "IP_COORDINADOR")
		&& config_has_property(fd_configuracion, "PUERTO_COORDINADOR")
		&& config_has_property(fd_configuracion, "CLAVES_INICIALMENTE_BLOQUEADAS"));
}

int cargarConfiguracion() {
	logger = log_create("LogPlanificador", "Planificador", true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(t_configuracion));

	//en eclipse cambia el path desde donde se corre, asi que probamos desde /Debug y desde /Planificador
	fd_configuracion = config_create("../Planificador.config");
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create("Planificador.config");
	}

	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.","ERROR");
		return -1;
	}

	configuracion->puerto = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA");
	char *algoritmo = config_get_string_value(fd_configuracion, "ALGORITMO_PLANIFICACION");
	if (strcmp(algoritmo, "SJF-CD") == 0) {
		configuracion->algoritmoPlanificacion = ALGORITMO_SJF_CD;
	} else if (strcmp(algoritmo, "SJF-SD") == 0) {
		configuracion->algoritmoPlanificacion = ALGORITMO_SJF_SD;
	} else if (strcmp(algoritmo, "HRRN") == 0) {
		configuracion->algoritmoPlanificacion = ALGORITMO_HRRN;
	}
	configuracion->estimacionInicial = config_get_int_value(fd_configuracion, "ESTIMACION_INICIAL");
	configuracion->ipCoordinador = config_get_string_value(fd_configuracion, "IP_COORDINADOR");
	configuracion->puertoCoordinador = config_get_int_value(fd_configuracion, "PUERTO_COORDINADOR");
	//ToDo: faltan las claves inicialmente bloqueadas

	log_info(logger,
		"\nPUERTO_ESCUCHA: %d\n"
		"ALGORITMO_PLANIFICACION: %d\n"
		"ESTIMACION_INICIAL: %d\n"
		"IP_COORDINADOR: %s\n"
		"PUERTO_COORDINADOR: %d\n" ,
		configuracion->puerto, configuracion->algoritmoPlanificacion , configuracion->estimacionInicial ,
		configuracion->ipCoordinador, configuracion->puertoCoordinador);
	return 0;
}

void limpiarConfiguracion() {
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);
}
