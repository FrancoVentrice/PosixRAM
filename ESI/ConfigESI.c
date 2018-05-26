#include"ESI.h"

int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "IP_COORDINADOR")
		&& config_has_property(fd_configuracion, "PUERTO_COORDINADOR")
		&& config_has_property(fd_configuracion, "IP_PLANIFICADOR")
		&& config_has_property(fd_configuracion, "PUERTO_PLANIFICADOR"));
}

int cargarConfiguracion() {
	logger = log_create("LogESI", "ESI", true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(t_configuracion));
	operacion = malloc(sizeof(t_operacionESI));
	lineptr = NULL;
	n = 0;
	lecturaRechazada = false;

	//en eclipse cambia el path desde donde se corre, asi que probamos desde /Debug y desde /ESI
	fd_configuracion = config_create("../ESI.conf");
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create("ESI.conf");
	}

	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.","ERROR");
		return -1;
	}

	configuracion->ipCoordinador = config_get_string_value(fd_configuracion, "IP_COORDINADOR");
	configuracion->puertoCoordinador = config_get_int_value(fd_configuracion, "PUERTO_COORDINADOR");
	configuracion->ipPlanificador = config_get_string_value(fd_configuracion, "IP_PLANIFICADOR");
	configuracion->puertoPlanificador = config_get_int_value(fd_configuracion, "PUERTO_PLANIFICADOR");

	log_info(logger,
		"\nIP_COORDINADOR: %s\n"
		"PUERTO_COORDINADOR: %d\n"
		"IP_PLANIFICADOR: %s\n"
		"PUERTO_PLANIFICADOR: %d\n",
		configuracion->ipCoordinador, configuracion->puertoCoordinador , configuracion->ipPlanificador ,
		configuracion->puertoPlanificador);
	return 0;
}

void limpiarConfiguracion() {
	fclose(archivo);
	free(lineptr);
	free(n);
	free(operacion);
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);
}
