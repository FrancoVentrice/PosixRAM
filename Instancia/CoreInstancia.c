/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

/* procesa los parámetros de línea de comandos */
int procesarLineaDeComandos (int argc, char *argv[]) {

	parametrosEntrada.debugMode = 0;
	parametrosEntrada.logPantalla = 0;

	parametrosEntrada.archivoConf = (char *)malloc(15);
	memset(parametrosEntrada.archivoConf, 0, 15);
	strcpy(parametrosEntrada.archivoConf,"Instancia.conf");

	for( int i = 0; i < argc; ++i ) {
		if (strcmp(argv[i], "--e") == 0) {
			// easter egg (error forzado)
			free(parametrosEntrada.archivoConf);
			printf("\nParámetros incorrectos.\nEjecute " AZUL_T "%s --help" RESET " para obtener más información.\n\n", argv[0]);
			return -1;
		}
		if (strcmp(argv[i], "--help") == 0) {
			printf("\nEjecución\n");
			printf("    ./Instancia [OPTION]\n");
			printf("Parámetros\n");
			printf(" " AZUL_T "--help" RESET "\t Muestra esta ayuda.\n");
			printf(" " AZUL_T "--d" RESET "\t Modo debug, setea el log con nivel LOG_LEVEL_TRACE.\n");
			printf(" " AZUL_T "--l" RESET "\t Indica que el log se debe mostrar en pantalla. Desactiva el modo gráfico.\n");
			printf(" " AZUL_T "--conf=FILE" RESET "\t Permite indicar un archivo de configuración. Ej.: ./Instancia --conf=InstUno.conf.\n\n");
			free(parametrosEntrada.archivoConf);
			exit(0);
		}
		if (strcmp(argv[i], "--d") == 0)
			parametrosEntrada.debugMode = 1;
		if (strcmp(argv[i], "--l") == 0) {
			parametrosEntrada.logPantalla = 1;
		}
		if (string_starts_with(argv[i], "--conf=")) {
			free(parametrosEntrada.archivoConf);
			parametrosEntrada.archivoConf = (char *)malloc(string_length(argv[i]));
			memset(parametrosEntrada.archivoConf, 0, string_length(argv[i]));
			strcpy(parametrosEntrada.archivoConf,string_substring_from(argv[i],7));
		}
	}

	return 1;
}

/* prepara el estado de la instancia */
void inicializarInstancia() {
	memset(configuracion.czNomProc, 0, 20);
	strcpy(configuracion.czNomProc,"InstanciaPosixRAM");
	configuracion.instruccionesProcesadas = 0;
}

/* inicia el logger para este proceso */
void iniciarLogger(){
	time_t tiempoActual;
	// Se obtiene el tiempo actual
	tiempoActual = time(NULL);
	char czFecha[10];
	// transforma los datos de fecha y hora a un formato de cadena
	strftime(czFecha, 10, "%Y%m%d", localtime(&tiempoActual));

	mkdir("./logs", ACCESSPERMS | S_IRWXU);

	char *nombreArchivoLog;
	nombreArchivoLog = (char *)malloc(50);
	memset(nombreArchivoLog,0,50);
	sprintf(nombreArchivoLog, "./logs/%s%d_%s.LOG", configuracion.czNomProc, process_getpid(), czFecha);

	if (parametrosEntrada.debugMode) {
		logger = log_create(nombreArchivoLog, configuracion.czNomProc, parametrosEntrada.logPantalla, LOG_LEVEL_TRACE);
		log_debug(logger,"Modo debug activado.");
	}
	else
		logger = log_create(nombreArchivoLog, configuracion.czNomProc, parametrosEntrada.logPantalla, LOG_LEVEL_INFO);

	/*
	log_trace(logger,"Nivel trace %d",LOG_LEVEL_TRACE);
	log_debug(logger,"Nivel debug %d",LOG_LEVEL_DEBUG);
	log_info(logger,"Nivel info %d",LOG_LEVEL_INFO);
	log_warning(logger,"Nivel waring %d",LOG_LEVEL_WARNING);
	log_error(logger,"Nivel error %d",LOG_LEVEL_ERROR);
	*/
	free(nombreArchivoLog);
}

/* termina el proceso correctamente liberando recursos */
void finalizar(int codigo) {
	alarm(0);
	if (fd_configuracion != NULL) {
		desconectarseDe(configuracion.fdSocketCoordinador);
		close(configuracion.fdTimerDump);
		log_info(logger,"Instancia %s finalizada" , configuracion.nombreDeInstancia);
		limpiarConfiguraion();
	}
	log_destroy(logger);
	free(parametrosEntrada.archivoConf);

	void limpiarTablaDeEntradas();

	pantallaFin();

	exit(codigo);
}
