#include"Planificador.h"

int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(fd_configuracion, "ALGORITMO_PLANIFICACION")
		&& config_has_property(fd_configuracion, "ESTIMACION_INICIAL")
		&& config_has_property(fd_configuracion, "IP_COORDINADOR")
		&& config_has_property(fd_configuracion, "PUERTO_COORDINADOR")
		&& config_has_property(fd_configuracion, "CLAVES_INICIALMENTE_BLOQUEADAS")
		&& config_has_property(fd_configuracion, "ALFA"));
}

void bloquearClavesIniciales() {
	char** claves = string_split(configuracion->clavesInicialmenteBloqueadas, ", ");
	int i = 0;
	while (true) {
		if (claves[i] != NULL) {
			bloquearClaveSola(claves[i]);
			i ++;
		} else  {
			break;
		}
	}
	free(claves);
}

int cargarConfiguracion() {
	logger = log_create("LogPlanificador", "Planificador", true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(t_configuracion));
	colaDeListos = list_create();
	colaDeFinalizados = list_create();
	diccionarioBloqueados = dictionary_create();

	//en eclipse cambia el path desde donde se corre, asi que probamos desde /Debug y desde /Planificador
	fd_configuracion = config_create("../Planificador.conf");
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create("Planificador.conf");
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
	int alfa = config_get_int_value(fd_configuracion, "ALFA");
	if (alfa < 0 || alfa > 100) {
		log_error(logger,"Valor de alfa inválido, asignando valor por defecto (50)","ERROR");
		configuracion->alfa = 50;
	} else {
		configuracion->alfa = alfa;
	}
	configuracion->clavesInicialmenteBloqueadas = config_get_string_value(fd_configuracion, "CLAVES_INICIALMENTE_BLOQUEADAS");

	bloquearClavesIniciales();

	log_info(logger,
		"\nPUERTO_ESCUCHA: %d\n"
		"ALGORITMO_PLANIFICACION: %d\n"
		"ESTIMACION_INICIAL: %d\n"
		"IP_COORDINADOR: %s\n"
		"PUERTO_COORDINADOR: %d\n"
		"ALFA: %d\n"
		"CLAVES_INICIALMENTE_BLOQUEADAS: %s\n",
		configuracion->puerto, configuracion->algoritmoPlanificacion , configuracion->estimacionInicial ,
		configuracion->ipCoordinador, configuracion->puertoCoordinador, configuracion->alfa,
		configuracion->clavesInicialmenteBloqueadas);
	return 0;
}

void limpiarConfiguracion() {
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);
	dictionary_destroy_and_destroy_elements(diccionarioBloqueados, esiDestroyer);
	list_destroy_and_destroy_elements(colaDeListos, esiDestroyer);
	list_destroy_and_destroy_elements(colaDeFinalizados, esiDestroyer);
	free(esiEnEjecucion);
}
