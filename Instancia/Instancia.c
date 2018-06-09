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

	if (!conectarACoordinador())
		finalizar(EXIT_FAILURE);
	/* se agrega el socket del coordinador */
	FD_SET(configuracion.fdSocketCoordinador,&master);
	if(iFdmax < configuracion.fdSocketCoordinador)
		iFdmax = configuracion.fdSocketCoordinador;
	mostrarConexionCoordinador();

	prepararTablaDeEntradas();
	if (!inicializarPuntoDeMontaje())
		finalizar(EXIT_FAILURE);
	mostrarEstadoTablaDeEntradas();

	// TODO sincronizar entradas con coordinador
	char ** entradasSincronizadas = string_split("clave_mas_de_100;Clave_de_100_bytes;Nuevo_archivo_con_Clave_de_Cuarenta_byte;clavedos;clave1punto1;clave1", ";");
	cargarEntradasDesdeArchivos(entradasSincronizadas);

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
			char *clave;
			char *valor;
			// TODO leer el socket
			// iBytesLeidos = LeerSocket();
			if (iBytesLeidos == 0) { // coordinador caído
				FD_CLR(configuracion.fdSocketCoordinador, &master);
				sigue = 0;
				fflush(stdin);
				fflush(stdout);
				log_warning(logger,"El coordinador dejó de responder.");
			}
			switch (tipoMensaje) {
			case C_EJECUTAR_SET:
				//ToDo: ejecutar una funcion que obtenga la posicion para guardar el valor
				//y cambiarla por el 0 en el llamado de ejecutarSet
				ejecutarSet(clave, valor, 0);
				enviarMensajeOK();
				break;
			case C_EJECUTAR_STORE:
				break;
			case C_EJECUTAR_COMPACTACION:
				break;
			}
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
				case 'L':
					// últimas 10 líneas del Log
				break;
				case 'R': // Refresh status
					pantallaInicio();
					mostrarConfiguracion();
					mostrarConexionCoordinador();
					mostrarEstadoTablaDeEntradas();
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
			iBytesLeidos = read(configuracion.fdTimerDump, &sBuf, sizeof(sBuf));
			volcarEntradas();
		}
		letra = '-';
	}

	finalizar(EXIT_SUCCESS);
}

void ejecutarSet(char *clave, char *valor, int posicion) {
	char * posicionValor;
	unsigned int i = posicion;

	    	posicionValor = almacenamientoEntradas + (i * configuracion.tamanioEntrada);
	    	//posicionValor = almacenamientoEntradas[i * configuracion.tamanioEntrada];

	    	strcpy(tablaDeEntradas[i].clave, clave);
	    	tablaDeEntradas[i].tamanio = string_length(valor);
	        memcpy(posicionValor, valor, tablaDeEntradas[i].tamanio);

	    	// unstable code: comparo un size_t con unsigned int pero... dale que va
	    	if (tablaDeEntradas[i].tamanio > configuracion.tamanioEntrada) {
	    		/* si el valor ocupa más de una entrada tengo que reflejarlo en la tabla */
	    		int entradasExtraOcupadas;
	    		entradasExtraOcupadas = tablaDeEntradas[i].tamanio / configuracion.tamanioEntrada;
	    		while (entradasExtraOcupadas) {
	    			i++;
	    			strcpy(tablaDeEntradas[i].clave,tablaDeEntradas[i-1].clave);
	    			entradasExtraOcupadas--;
	    		}
	    	}
}

void enviarMensajeOK() {
	tPaquete pkgSetOk;
	int bytesEnviados;
	char* lineaOk = malloc(5);
	strcpy(lineaOk, "OK");
	pkgSetOk.type = I_RESULTADO_SET;

	pkgSetOk.length = serializar(pkgSetOk.payload, "%s", lineaOk);

	log_info(logger, "Se envia %s al Planificador", lineaOk);
	bytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador,
			&pkgSetOk, logger, "Se envia OK al Planificador");
	log_info(logger, "Se envian %d bytes", bytesEnviados);
	free(lineaOk);
}
