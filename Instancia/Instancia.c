/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

int main(int argc, char *argv[]) {
	int iFdmax = 0;
	int iBytesLeidos;
	fd_set master;
	fd_set read_fds;
	char letra = 45;

	// controlar argumentos de entrada
	if (procesarLineaDeComandos(argc, argv) < 0) {
		printf("\nParámetros incorrectos.\nEjecute \033[1m\033[37m %s --help \033[0m para obtener más información.\n\n", argv[0]);
		return EXIT_FAILURE;
	}

	// preparando proceso
	memset(czNomProc, 0, 20);
	strcpy(czNomProc,"InstanciaPosixRAM");

	pantallaInicio ();

	iniciarLogger();
	log_info(logger,"Iniciando Instancia PosixRAM para ReDistinto");

	if(!cargarConfiguracion())
		finalizar(EXIT_FAILURE);
	mostrarConfiguracion();

	if(!conectarACoordinador())
		finalizar(EXIT_FAILURE);

	// TODO armar estructura de entradas

	// TODO preparar punto de montaje

	// iniciamos el timeout para el vuelco seteando una alarma
	iniciarDumpTimeout();

	/* se limpian la estructuras */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	/* se agrega el socket de teclado */
	FD_SET(STDIN,&master);

	/* se agrega el socket de escucha para habitacion*/
	FD_SET(configuracion->fdSocketCoordinador,&master);
	if(iFdmax < configuracion->fdSocketCoordinador)
		iFdmax = configuracion->fdSocketCoordinador;

	// while principal
	int sigue = 1;
	while(sigue) {
		read_fds = master;

		if(select(iFdmax+1,&read_fds,NULL,NULL,NULL)==-1)
			log_error(logger,"Falló función select().");

		if(FD_ISSET(configuracion->fdSocketCoordinador, &read_fds)) {
			// TODO leer el socket
			// iBytesLeidos = LeerSocket();
			if (iBytesLeidos == 0) { // coordinador caído
				FD_CLR(configuracion->fdSocketCoordinador, &master);
				sigue = 0;
				fflush(stdin);
				fflush(stdout);
				log_warning(logger,"El coordinador dejó de responder.");
			}
		}

		/* si tengo algo en el teclado */
		if (FD_ISSET(STDIN,&read_fds)) {
			scanf("%c", &letra);
			letra = toupper(letra);

			switch (letra) {
				case 'D':
					// forzar dump
				break;
				case 'Q': // quit (salir)
					sigue = 0;
				break;
				default:
					letra = '-';
				break;
			}
		}

		if (realizarDump) {
			volcarEntradas();
		}
		letra = '-';
	}

	finalizar(EXIT_SUCCESS);
}

/* procesa los parámetros de línea de comandos */
int procesarLineaDeComandos (int argc, char *argv[]) {

	parametrosEntrada = malloc(sizeof(t_commandLineParameters));
	parametrosEntrada->debugMode = 0;
	parametrosEntrada->logPantalla = 0;

	parametrosEntrada->archivoConf = (char *)malloc(15);
	memset(parametrosEntrada->archivoConf, 0, 15);
	strcpy(parametrosEntrada->archivoConf,"Instancia.conf");

	for( int i = 0; i < argc; ++i ) {
		if (strcmp(argv[i], "--e") == 0) {
			// easter egg (error forzado)
			free(parametrosEntrada->archivoConf);
			free(parametrosEntrada);
			return -1;
		}
		if (strcmp(argv[i], "--help") == 0) {
			printf("\nEjecución\n");
			printf("    ./Instancia [OPTION]\n");
			printf("Parámetros\n");
			printf("\033[1m\033[37m --help \033[0m Muestra esta ayuda.\n");
			printf("\033[1m\033[37m --d \033[0m Modo debug, setea el log con nivel LOG_LEVEL_TRACE.\n");
			printf("\033[1m\033[37m --l \033[0m Indica que el log se debe mostrar en pantalla. Desactiva el modo gráfico.\n");
			printf("\033[1m\033[37m --conf=FILE \033[0m Permite indicar un archivo de configuración. Ej.: ./Instancia --conf=InstUno.conf.\n\n");
			free(parametrosEntrada->archivoConf);
			free(parametrosEntrada);
			exit(0);
		}
		if (strcmp(argv[i], "--d") == 0)
			parametrosEntrada->debugMode = 1;
		if (strcmp(argv[i], "--l") == 0) {
			parametrosEntrada->logPantalla = 1;
		}
		if (string_starts_with(argv[i], "--conf=")) {
			free(parametrosEntrada->archivoConf);
			parametrosEntrada->archivoConf = (char *)malloc(string_length(argv[i]));
			memset(parametrosEntrada->archivoConf, 0, string_length(argv[i]));
			strcpy(parametrosEntrada->archivoConf,string_substring_from(argv[i],7));
		}
	}

	return 1;
}

/* termina el proceso correctamente liberando recursos */
void finalizar(int codigo) {
	alarm(0);
	desconectarseDe(configuracion->fdSocketCoordinador);
	log_info(logger,"Instancia %s finalizada" , configuracion->nombreDeInstancia);
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);
	free(parametrosEntrada->archivoConf);
	free(parametrosEntrada);

	pantallaFin();

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

	if (parametrosEntrada->debugMode) {
		logger = log_create(nombreArchivoLog, czNomProc, parametrosEntrada->logPantalla, LOG_LEVEL_TRACE);
		log_debug(logger,"Modo debug activado.");
	}
	else
		logger = log_create(nombreArchivoLog, czNomProc, parametrosEntrada->logPantalla, LOG_LEVEL_INFO);

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
int cargarConfiguracion() {
	log_info(logger,"Cargando archivo de configuración: %s", parametrosEntrada->archivoConf);

	configuracion = malloc(sizeof(t_confInstancia));

	fd_configuracion = config_create(parametrosEntrada->archivoConf);
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

/* timeout para disparar señal de dump */
void iniciarDumpTimeout() {
	realizarDump = 0;
	signal(SIGALRM, capturaSenial);
	alarm(configuracion->intervaloDump);
	log_info(logger,"Seteada alarma para vuelco en %d segundos.",configuracion->intervaloDump);
}

/* handler de señales */
void capturaSenial(int iSignal) {
	switch(iSignal) {
		case SIGINT:
			// interrupción del teclado no se está tratando
		break;
		case SIGTERM:
			// terminación del programa no se está tratando
		break;
		case SIGCHLD:
			signal(SIGCHLD,SIG_IGN);
			while(waitpid(WAIT_MYPGRP,NULL,WNOHANG)>0)
			signal(SIGCHLD,capturaSenial);
		break;
		case SIGALRM:
			signal(SIGALRM, SIG_IGN);
			alarm(0);
			realizarDump = 1;
		break;
	}
}

/* proceso de dump */
void volcarEntradas() {
	// TODO completar este proceso
	log_info(logger,"Ejecutando proceso de vuelco...");
	retardoSegundos(3);
	log_info(logger,"Vuelco finalizado.");
	iniciarDumpTimeout();
}

/* conecta con el coordinador y hace el handshake */
int conectarACoordinador() {
	log_info(logger,"Conectando con el Coordinador (IP: %s Puerto: %d)...", configuracion->ipCoordinador,configuracion->puertoCoordinador);
	configuracion->fdSocketCoordinador = connectToServer(configuracion->ipCoordinador,configuracion->puertoCoordinador, logger);
	if (configuracion->fdSocketCoordinador < 0) {
		log_error(logger,"No se pudo conectar con el Coordinador.");
		mostrarTexto("ERROR: No se pudo conectar con el Coordinador.");
		return 0;
	}
	log_info(logger,"Conexión exitosa con el Coordinador.");

	// handshake con coordinador
	log_info(logger,"Realizando handshake con el Coordinador.");
	tPaquete pkgHandshake;
	pkgHandshake.type = I_HANDSHAKE;
	pkgHandshake.length = serializar(pkgHandshake.payload, "%s",configuracion->nombreDeInstancia);
	enviarPaquete(configuracion->fdSocketCoordinador, &pkgHandshake, logger,"Enviando Handshake...");

	char * sPayloadRespuesta = (char *)malloc(50);
	char * mensajeRespuesta = (char *)malloc(50);
	tMensaje tipoMensaje;
	recibirPaquete(configuracion->fdSocketCoordinador, &tipoMensaje, &sPayloadRespuesta, logger, "Recibiendo respuesta de coordinador...");
	deserializar(sPayloadRespuesta, "%d;%d;%s", &(configuracion->cantidadEntradas), &(configuracion->tamanioEntrada), mensajeRespuesta);
	free(sPayloadRespuesta);

	if (!(configuracion->cantidadEntradas * configuracion->tamanioEntrada)) {
		log_error(logger,"Handshake no completado con el Coordinador: %s", mensajeRespuesta);
		mostrarTexto("ERROR: Handshake no completado con el Coordinador");
		mostrarTexto(mensajeRespuesta);
		free(mensajeRespuesta);
		return 0;
	}
	log_info(logger,"Handshake exitoso con el Coordinador: %s", mensajeRespuesta);
	free(mensajeRespuesta);

	return 1;
}

void pantallaInicio() {
	limpiarPantalla();
	printf("\e[36m");
	centrarTexto("Instancia PosixRAM para ReDistinto");
	centrarTexto("==================================");
	printf("\e[0m");
}

void mostrarConfiguracion() {
	if(parametrosEntrada->logPantalla)
		return;

	printf("\nArchivo de configuración cargado:\033[1m\033[37m %s\033[0m", parametrosEntrada->archivoConf);
	printf("\n - Instancia:\033[1m\033[37m %s\033[0m", configuracion->nombreDeInstancia);
	printf("\n - Algoritmo de reemplazo:\033[1m\033[37m %s\033[0m", config_get_string_value(fd_configuracion, "ALGORITMO_REEMPLAZO"));
	printf("\n - Punto de montaje:\033[1m\033[37m %s\033[0m", configuracion->puntoDeMontaje);
	printf("\n - Intervalo para dump:\033[1m\033[37m %d segundos\033[0m\n", configuracion->intervaloDump);
}

void pantallaFin() {
	printf("\e[33m\n");
	centrarTexto("PosixRAM (c) 2018");
	printf("\e[0m\n");
}

void mostrarTexto(char *cadena) {
	if(parametrosEntrada->logPantalla)
		return;

	puts(cadena);
}
