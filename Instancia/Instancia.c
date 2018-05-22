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

	inicializarInstancia();

	pantallaInicio ();

	iniciarLogger();
	log_info(logger,"Iniciando Instancia PosixRAM para ReDistinto");

	if(!cargarConfiguracion())
		finalizar(EXIT_FAILURE);
	mostrarConfiguracion();

	if(!conectarACoordinador())
		finalizar(EXIT_FAILURE);

	// TODO preparar punto de montaje

	// TODO armar estructura de entradas

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
				case 'C':
					// forzar Compactación
				break;
				case 'D':
					// forzar Dump
				break;
				case 'E':
					// listar Entradas
				break;
				case 'L':
					// últimas 10 líneas del Log
				break;
				case 'R':
					// Refresh status
				break;
				case 'Q': // Quit (salir)
					sigue = 0;
				break;
				default:
					letra = '-';
				break;
			}
		}

		if (estadoInstancia.realizarDump) {
			volcarEntradas();
		}
		letra = '-';
	}

	finalizar(EXIT_SUCCESS);
}

/* timeout para disparar señal de dump */
void iniciarDumpTimeout() {
	estadoInstancia.realizarDump = 0;
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
			signal(SIGCHLD, SIG_IGN);
			while(waitpid(WAIT_MYPGRP, NULL, WNOHANG) > 0)
			signal(SIGCHLD, capturaSenial);
		break;
		case SIGALRM:
			signal(SIGALRM, SIG_IGN);
			alarm(0);
			estadoInstancia.realizarDump = 1;
		break;
	}
}

/* proceso de dump */
void volcarEntradas() {
	// TODO completar este proceso
	log_info(logger,"Ejecutando proceso de vuelco...");
	retardoSegundos(3);
	estadoInstancia.ultimoDump = time(NULL);
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
