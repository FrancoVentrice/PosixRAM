/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

/* carga el archivo de configuracion default */
int cargarConfiguracion() {
	log_info(logger,"Cargando archivo de configuración: %s", parametrosEntrada.archivoConf);

	configuracion = malloc(sizeof(t_confInstancia));

	fd_configuracion = config_create(parametrosEntrada.archivoConf);
	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.");
		mostrarTexto("Error al cargar el archivo de configuración.");
		return 0;
	}

	configuracion->ipCoordinador = config_get_string_value(fd_configuracion, "IP_COORDINADOR");
	configuracion->puertoCoordinador = config_get_int_value(fd_configuracion, "PUERTO_COORDINADOR");

	char *algoritmo = config_get_string_value(fd_configuracion, "ALGORITMO_REEMPLAZO");
	// lo que sigue es necesario porque C no tiene un switch de strings ::facepalm::
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

	configuracion->fdSocketCoordinador = -1;
	configuracion->cantidadEntradas = 0;
	configuracion->tamanioEntrada = 0;

	log_info(logger,"Configuración cargada correctamente.");
	log_info(logger," - Instancia: %s", configuracion->nombreDeInstancia);
	log_info(logger," - Algoritmo de reemplazo: %s", algoritmo);
	log_info(logger," - Punto de montaje: %s", configuracion->puntoDeMontaje);
	log_info(logger," - Intervalo para dump: %d segundos", configuracion->intervaloDump);
	return 1;
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