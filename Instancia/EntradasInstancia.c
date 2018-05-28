/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

void prepararTablaDeEntradas() {
	/* prepara la estructura de la tabla de entradas y asigna el espacio para almacenar los valores */

	unsigned int espacioTotal;
	int i;

	espacioTotal = (configuracion->cantidadEntradas) * (configuracion->tamanioEntrada);
	log_info(logger,"Reservando espacio de almacenamiento para %d bytes.",espacioTotal);
	almacenamientoEntradas = (char *)malloc(espacioTotal);
	memset(almacenamientoEntradas, 0, espacioTotal);

	log_info(logger,"Preparando tabla para %d entradas.",configuracion->cantidadEntradas);
	tablaDeEntradas = (t_entrada *)malloc((configuracion->cantidadEntradas) * sizeof(t_entrada));

	for (i=0 ; i < configuracion->cantidadEntradas ; i++) {
		log_debug(logger,"... seteando entrada %d",i);
		memset(tablaDeEntradas[i].clave, 0, MAX_LONG_CLAVE);
		tablaDeEntradas[i].tamanio = 0;
		tablaDeEntradas[i].ultimaInstruccion = 0;
	}
}

unsigned int espacioDisponible() {
	/* calcula el espacio disponible según las entradas ocupadas */

	unsigned int espacioTotal;
	int i;

	log_debug(logger,"Calculando espacio disponible");
	espacioTotal = (configuracion->cantidadEntradas) * (configuracion->tamanioEntrada);

	for (i=0 ; i < configuracion->cantidadEntradas ; i++)
		espacioTotal = espacioTotal - tablaDeEntradas[i].tamanio;

	return espacioTotal;
}

void iniciarDumpTimeout() {
	/* timeout para disparar señal de dump */

	estadoInstancia.realizarDump = 0;
	signal(SIGALRM, capturaSenial);
	alarm(configuracion->intervaloDump);
	log_info(logger,"Seteada alarma para vuelco en %d segundos.",configuracion->intervaloDump);
}

void volcarEntradas() {
	/* proceso de dump */

	// TODO completar este proceso
	log_info(logger,"Ejecutando proceso de vuelco...");
	retardoSegundos(3);
	estadoInstancia.ultimoDump = time(NULL);
	log_info(logger,"Vuelco finalizado.");
	iniciarDumpTimeout();
}


void limpiarTablaDeEntradas() {
	/* libera la entrada de entradas y el espacio de almacenamiento reservado */

	if (almacenamientoEntradas != NULL) {
		free(almacenamientoEntradas);
		free(tablaDeEntradas);
	}
}
