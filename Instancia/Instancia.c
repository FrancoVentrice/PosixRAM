/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include <time.h>
#include <sys/stat.h> // mkdir
#include "Instancia.h"
#include "..//shared/libgral.h"
#include "..//shared/sockets.h"

int main(int argn, char *argv[]) {
	// preparando proceso
	memset(czNomProc, 0, 20);
	strcpy(czNomProc,"InstanciaPosixRAM");

	limpiarPantalla();
	printf("\e[36m");
	centrarTexto("Instancia PosixRAM para ReDistinto");
	centrarTexto("==================================");
	printf("\e[0m");

	iniciarLogger();
	log_info(logger,"Iniciando Instancia PosixRAM para ReDistinto");

	// se chequean los argumentos para levantar la configuración
	if(argn==2) {
		// un argumento, debería ser el nombre del archivo de configuración
		if(!cargarConfiguracion(argv[1]))
			return -1;
	}
	else if(argn==7) {
		/* 6 argumentos, se podría levantar una instancia sin archivo, con los datos por línea de comandos
		 TODO completar esta forma de ejecutarse si se considera necesario */
	} else {
		/* argn==1 (sin parámetros)
		   argn!=2 && argn!=7 (cualquier otra variante)
		 se carga configuración desde archivo default */
		//cargarConfiguracion(DEFAULT_CONFIG_FILE);
		if(!cargarConfiguracion("Instancia.conf"))
			return -1;
	}

	// TODO conectar con coordinador

	/* TODO
	 * handshake coordinador
	 * enviar nombre para que valide unicidad
	 * si está ok recibe cantidad de entradas y tamaño
	 */

	// TODO armar estructura de entradas

	// TODO preparar proceso de Dump (configuraion->intervaloDump)

	// TODO while principal
	int sigue=1;
	while(sigue) {
		sigue = 0;
		retardoSegundos(15);
	}

	finalizar(EXIT_SUCCESS);
}

/* termina el proceso correctamente liberando recursos */
void finalizar(int codigo) {
	desconectarseDe(configuracion->fdSocketCoordinador);
	log_info(logger,"Instancia %s finalizada" , configuracion->nombreDeInstancia);
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);

	// imprimo mensaje de salida
	printf("\e[33m\n");
	centrarTexto("Instancia finalizada");
	centrarTexto("PosixRAM (c) 2018");
	printf("\e[0m\n");

	exit(codigo);
}

/* inicia el logger para este proceso */
void iniciarLogger(){
	time_t tiempoActual;
	// Se obtiene el tiempo actual
	tiempoActual = time(NULL);
	char czFecha[10];
	// transforma los datos de fecha y hora a un formato de cadena
	strftime(czFecha, 10, "%Y%m%d", localtime(&tiempoActual));

	mkdir("./logs",0755);

	char *nombreArchivoLog;
	nombreArchivoLog = (char *)malloc(50);
	memset(nombreArchivoLog,0,50);
	sprintf(nombreArchivoLog, "./logs/%s%d_%s.LOG", czNomProc, process_getpid(), czFecha);

	logger = log_create(nombreArchivoLog, czNomProc, true, LOG_LEVEL_INFO);
	/*
	log_trace(logger,"Nivel trace %d",LOG_LEVEL_TRACE);
	log_debug(logger,"Nivel debug %d",LOG_LEVEL_DEBUG);
	log_info(logger,"Nivel info %d",LOG_LEVEL_INFO);
	log_warning(logger,"Nivel waring %d",LOG_LEVEL_WARNING);
	log_error(logger,"Nivel error %d",LOG_LEVEL_ERROR);
	*/
	free(nombreArchivoLog);
}

/* carga el archivo de configuracion default */
int cargarConfiguracion(char *configFilePath) {
	log_info(logger,"Cargando archivo de configuración: %s", configFilePath);

	configuracion = malloc(sizeof(t_confInstancia));

	fd_configuracion = config_create(configFilePath);
	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.");
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
