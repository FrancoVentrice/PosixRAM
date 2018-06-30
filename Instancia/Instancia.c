/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

int main(int argc, char *argv[]) {
	int iFdmax = 0;
	char * clavesSincronizadas;
	fd_set master;
	fd_set read_fds;
	char letra = 45;

	// controlar argumentos de entrada
	if (procesarLineaDeComandos(argc, argv) < 0) {
		return EXIT_FAILURE;
	}

	inicializarInstancia();
	/* se limpian la estructuras de sockets */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	/* se agrega el socket de teclado */
	FD_SET(STDIN,&master);

	pantallaInicio();

	iniciarLogger();
	log_info(logger,"Iniciando Instancia PosixRAM para ReDistinto");

	if (!cargarConfiguracion())
		finalizar(EXIT_FAILURE);
	mostrarConfiguracion();

	clavesSincronizadas = string_new();
	if (!conectarACoordinador(&clavesSincronizadas))
		finalizar(EXIT_FAILURE);
	/* se agrega el socket del coordinador */
	FD_SET(configuracion.fdSocketCoordinador,&master);
	if(iFdmax < configuracion.fdSocketCoordinador)
		iFdmax = configuracion.fdSocketCoordinador;
	mostrarConexionCoordinador();

	prepararTablaDeEntradas();
	if (!inicializarPuntoDeMontaje())
		finalizar(EXIT_FAILURE);
	// un mundo feliz: estoy asumiendo que no hay errores en la siguiente función
	procesarClavesYCargarEntradas(clavesSincronizadas);
	mostrarEstadoTablaDeEntradas();

	/* iniciamos el timeout para el vuelco seteando un timer */
	iniciarDumpTimeout();
	/* se agrega el file descriptor del timeout para el vuelco */
	FD_SET(configuracion.fdTimerDump,&master);
	if(iFdmax < configuracion.fdTimerDump)
		iFdmax = configuracion.fdTimerDump;

	mostrarMenu();

	// while principal
	int sigue = 1;
	while(sigue) {
		read_fds = master;

		if(select(iFdmax+1,&read_fds,NULL,NULL,NULL)==-1)
			log_error(logger,"Falló función select().");

		/* si hay algo en el socket de coordinador */
		if(FD_ISSET(configuracion.fdSocketCoordinador, &read_fds)) {
			tMensaje tipoMensaje;
			int iBytesLeidos = 0;
			int iBytesEnviados;
			char * claveRecibida;
			char * valorRecibido;

			char * sPayloadRespuesta = (char *)calloc(configuracion.cantidadEntradas, configuracion.tamanioEntrada);

			iBytesLeidos = recibirPaquete(configuracion.fdSocketCoordinador, &tipoMensaje, &sPayloadRespuesta, logger, "Recibiendo mensaje desde Coordinador.");

			if (iBytesLeidos == 0) { // coordinador caído
				FD_CLR(configuracion.fdSocketCoordinador, &master);
				sigue = 0;
				fflush(stdin);
				fflush(stdout);
				log_warning(logger,"El coordinador dejó de responder.");
			}
			configuracion.instruccionesProcesadas++;
			switch (tipoMensaje) {
				case C_EJECUTAR_SET:
					log_info(logger,"Recibido mensaje del Coordinador C_EJECUTAR_SET (%d).",C_EJECUTAR_SET);
					claveRecibida = (char *)malloc(MAX_LONG_CLAVE);
					valorRecibido = (char *)malloc(iBytesLeidos);
					memset(valorRecibido,0,iBytesLeidos);

					deserializar(sPayloadRespuesta, "%s%s", claveRecibida, valorRecibido);


					//ToDo: ejecutar una funcion que obtenga la posicion para guardar el valor
					//y cambiarla por el 0 en el llamado de ejecutarSet
					//  deprecated_ejecutarSet(claveRecibida, valorRecibido, 0);
					// deprecated_enviarMensajeOK();
					// *************************************************************************
					// ToDo: lo que sigue es la respuesta
					// *************************************************************************
					tPaquete pkgResultadoSet;
					t_respuestaSet respuestaSet;

					strcpy(respuestaSet.claveReemplazada,claveRecibida);
					respuestaSet.compactacionRequerida = 0;

					pkgResultadoSet.type = I_RESULTADO_SET;
					pkgResultadoSet.length = serializar(pkgResultadoSet.payload, "%d%s%c", entradasDisponibles(), respuestaSet.claveReemplazada, respuestaSet.compactacionRequerida);

					iBytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgResultadoSet, logger, "Se envia OK al Planificador");
					// *************************************************************************
					// *************************************************************************

					free(claveRecibida);
					free(valorRecibido);
					//tablaDeEntradas[indiceClave].ultimaInstruccion = configuracion.instruccionesProcesadas;
					break;

				case C_EJECUTAR_STORE:
					log_info(logger,"Recibido mensaje del Coordinador C_EJECUTAR_STORE (%d).",C_EJECUTAR_STORE);
					iBytesEnviados = atenderStoreClave(sPayloadRespuesta);
					break;

				case C_EJECUTAR_COMPACTACION:
					log_info(logger,"Recibido mensaje del Coordinador C_EJECUTAR_COMPACTACION (%d).",C_EJECUTAR_COMPACTACION);
					iBytesEnviados = atenderEjecutarCompactacion();
					break;

				case C_ESTADO_CLAVE:
					log_info(logger,"Recibido mensaje del Coordinador C_ESTADO_CLAVE (%d).",C_ESTADO_CLAVE);
					iBytesEnviados = atenderEstadoClave(sPayloadRespuesta);
					break;

				default:
					log_warning(logger,"Se recibió tipo de mensaje inválido: %d",tipoMensaje);
					break;
			}
			free(sPayloadRespuesta);
			log_debug(logger, "Se enviaron %d bytes", iBytesEnviados);
		}

		/* si hay algo en el teclado */
		if (FD_ISSET(STDIN,&read_fds)) {
			scanf("%c", &letra);
			letra = toupper(letra);

			switch (letra) {
				case 'C': // forzar Compactación
					realizarCompactacion();
				break;
				case 'D': // forzar Dump
					volcarEntradasEnArchivos();
				break;
				case 'E': // listar Entradas
					listarEntradas();
				break;
				case 'R': // Refresh screen
					refrescarPantalla();
				break;
				case 'Q': // Quit (salir)
					sigue = 0;
				break;
				default:
					letra = '-';
				break;
			}
		}

		/* si se activó el timeout del vuelco */
		if (FD_ISSET(configuracion.fdTimerDump,&read_fds)) {
			uint64_t sBuf = 0;
			int iBytesLeidos = 0;
			iBytesLeidos = read(configuracion.fdTimerDump, &sBuf, sizeof(uint64_t));
			log_debug(logger,"Se leyeron %d bytes del timer",iBytesLeidos);
			volcarEntradasEnArchivos();
		}
		letra = '-';
	}

	finalizar(EXIT_SUCCESS);
}
