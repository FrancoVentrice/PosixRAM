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
				free(sPayloadRespuesta);
				fflush(stdin);
				fflush(stdout);
				log_warning(logger,"El coordinador dejó de responder.");
			}
			switch (tipoMensaje) {
				case C_EJECUTAR_SET:
					claveRecibida = (char *)malloc(MAX_LONG_CLAVE);
					valorRecibido = (char *)malloc(iBytesLeidos);
					memset(valorRecibido,0,iBytesLeidos);

					deserializar(sPayloadRespuesta, "%s%s", claveRecibida, valorRecibido);
					free(sPayloadRespuesta);

					configuracion.instruccionesProcesadas++;

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
					break;
				case C_EJECUTAR_STORE:
					claveRecibida = (char *)malloc(MAX_LONG_CLAVE);

					deserializar(sPayloadRespuesta, "%s", claveRecibida);
					free(sPayloadRespuesta);

					// *************************************************************************
					// ToDo: lo que sigue es la respuesta
					// *************************************************************************
					tPaquete pkgResultadoStore;

					pkgResultadoStore.type = I_RESULTADO_STORE;
					pkgResultadoStore.length = serializar(pkgResultadoStore.payload, "", NULL);

					iBytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgResultadoStore, logger, "Se envia OK al Planificador");
					// *************************************************************************
					// *************************************************************************

					free(claveRecibida);
					break;
				case C_EJECUTAR_COMPACTACION:
					log_debug(logger, "esta línea evita el <a label can only be part of a statement and a declaration is not a statement>");
					// *************************************************************************
					// ToDo: lo que sigue es la respuesta
					// *************************************************************************
					tPaquete pkgResultadoCompactacion;

					pkgResultadoCompactacion.type = I_COMPACTACION_TERMINADA;
					pkgResultadoCompactacion.length = serializar(pkgResultadoCompactacion.payload, "", NULL);

					iBytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgResultadoCompactacion, logger, "Se envia OK al Planificador");
					// *************************************************************************
					// *************************************************************************
					break;
				case C_ESTADO_CLAVE:
					claveRecibida = (char *)malloc(MAX_LONG_CLAVE);

					deserializar(sPayloadRespuesta, "%s", claveRecibida);
					free(sPayloadRespuesta);
					// *************************************************************************
					// ToDo: lo que sigue es la respuesta
					// *************************************************************************
					tPaquete pkgResultadoEstadoClave;

					pkgResultadoEstadoClave.type = I_ESTADO_CLAVE;
					pkgResultadoEstadoClave.length = serializar(pkgResultadoEstadoClave.payload, "%s", "valor hardcodeado :D");

					iBytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgResultadoEstadoClave, logger, "Se envia OK al Planificador");
					// *************************************************************************
					// *************************************************************************

					free(claveRecibida);
					break;
				default:
					log_warning(logger,"Se recibió tipo de mensaje inválido: %d",tipoMensaje);
					break;
			}
			log_debug(logger, "Se enviaron %d bytes", iBytesEnviados);
		}

		/* si hay algo en el teclado */
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
				case 'E': // listar Entradas
					listarEntradas();
				break;
				case 'R': // Refresh status
					pantallaInicio();
					mostrarConfiguracion();
					mostrarConexionCoordinador();
					mostrarEstadoTablaDeEntradas();
					mostrarMenu();
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
			size_t sBuf = 0;
			read(configuracion.fdTimerDump, &sBuf, sizeof(sBuf));
			volcarEntradas();
		}
		letra = '-';
	}

	finalizar(EXIT_SUCCESS);
}
