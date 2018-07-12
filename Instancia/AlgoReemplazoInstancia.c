/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

/* Los algoritmos de reemplazo simplemente liberan la cantidad de entradas necesarias
 * para que un nuevo valor pueda ser almacenado, no "reemplazan" efectivamente sino
 * que disponibilizan las entradas para que se haga el reemplazo.
 *
 * Al estar basados solamente en entradas atómicas, es más sencillo su manejo.*/


// TODO agregar los logs que hagan falta

#include "Instancia.h"

bool existenEntradasAtomicasParaReemplazar(int cantEntradas) {
	/* Busca si existen entradas atómicas suficientes para aplicar el algoritmo de reemplazo completo.
	 * Esto puede ser una sobrecarga, pero está pensado para que la acción de reemplazo sea atómica
	 * y no se reemplace una entrada y se corte, por no haber más reemplazos posibles, en los casos
	 * en que se requieren más de una. */
	int cantEntradasAtomicas = 0;
	int i = 0;

	while (i < configuracion.cantidadEntradas && cantEntradasAtomicas < cantEntradas) {
		if(esEntradaAtomica(i))
			cantEntradasAtomicas++;
		i++;
	}

	return cantEntradasAtomicas >= cantEntradas;
}

void ejecutarReemplazo(int cantEntradas) {
	/* Decide qué algoritmo de reemplazo aplicar y lo dispara.
	 * Los algoritmos no reemplazan efectivamente una entrada, sino que liberan el lugar. */

	switch(configuracion.algoritmoDeReemplazo) {
		case ALGORITMO_CIRC:
			reemplazoCircular(cantEntradas);
			break;
		case ALGORITMO_LRU:
			reemplazoLRU(cantEntradas);
			break;
		case ALGORITMO_BSU:
			reemplazoBSU(cantEntradas);
			break;
	}
}

void reemplazoBSU(int cantEntradas) {
	/* Biggest Space Used */

	int cantLiberadas = 0;
	int i;
	int indiceBSU;
	size_t tamanioBSU;

	while (cantLiberadas < cantEntradas) {
		tamanioBSU = 0;

		// ToDo atención acá!! si hay más de 1 con el máximo, no estoy usando circular para definir, sino que se queda con el primero
		for(i=0; i < configuracion.cantidadEntradas ; i++) {
			if(esEntradaAtomica(i) && tablaDeEntradas[i].tamanio > tamanioBSU) {
				indiceBSU = i;
				tamanioBSU = tablaDeEntradas[i].tamanio;
			}
		}

		memset(tablaDeEntradas[indiceBSU].clave, 0, MAX_LONG_CLAVE);
		tablaDeEntradas[indiceBSU].tamanio = (size_t) 0;
		tablaDeEntradas[indiceBSU].ultimaInstruccion = 0;

		cantLiberadas++;
	}
}

void reemplazoLRU(int cantEntradas) {
	/* Least Recently Used */

	int cantLiberadas = 0;
	int i;
	int indiceLRU;
	unsigned int instruccionLRU;

	while (cantLiberadas < cantEntradas) {
		instruccionLRU = configuracion.instruccionesProcesadas;

		for(i=0; i < configuracion.cantidadEntradas ; i++) {
			if(esEntradaAtomica(i) && tablaDeEntradas[i].ultimaInstruccion < instruccionLRU) {
				indiceLRU = i;
				instruccionLRU = tablaDeEntradas[i].ultimaInstruccion;
			}
		}

		memset(tablaDeEntradas[indiceLRU].clave, 0, MAX_LONG_CLAVE);
		tablaDeEntradas[indiceLRU].tamanio = (size_t) 0;
		tablaDeEntradas[indiceLRU].ultimaInstruccion = 0;

		cantLiberadas++;
	}
}

void reemplazoCircular(int cantEntradas) {
	/* Circular */

	int cantLiberadas = 0;
	int i;

	i = configuracion.indiceCIRC;

	while (cantLiberadas < cantEntradas) {
		if(esEntradaAtomica(i)) {

			memset(tablaDeEntradas[i].clave, 0, MAX_LONG_CLAVE);
			tablaDeEntradas[i].tamanio = (size_t) 0;
			tablaDeEntradas[i].ultimaInstruccion = 0;

			cantLiberadas++;
		}
		i++;
		if(i == configuracion.cantidadEntradas)
			i = 0;
	}
	configuracion.indiceCIRC = i;
}
