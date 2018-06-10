
#include "Instancia.h"

int esAtomica(t_entrada* entrada){

	if(entrada->tamanio<=configuracion->tamanioEntrada){
		return 1;
	}else return 0;
}

int cantElementosTabla(t_entrada* tablaEntradas){

	return sizeof(tablaEntradas)/sizeof(tablaEntradas[0]);
}
S
int reemplazoLRU() {

	int i;
	int posReemplazo = 0;
	int cantEntradas = cantElementosTabla(tablaDeEntradas);
	t_entrada* entradaReemplazo = tablaDeEntradas[0];

	for (i = 0; i < cantEntradas; i++) {

		t_entrada* entradaActual = tablaDeEntradas[i];
		if(entradaActual->ultimaInstruccion>=entradaReemplazo->ultimaInstruccion){
			entradaReemplazo=entradaActual;
			posReemplazo=i;
		}

	}

	return posReemplazo;

}

int reemplazoBSU(t_list* entradas){
	int i;
	int posReemplazo = 0;
	int cantEntradas = cantElementosTabla;
	t_entrada* entradaReemplazo = tablaDeEntradas[0];

	for (i = 0; i < cantEntradas; i++) {
		if(esAtomica(tablaDeEntradas[i])){
			if(tablaDeEntradas[i].tamanio>=entradaReemplazo->tamanio){
				posReemplazo=i;
			}

		}

	}
	return posReemplazo;

}

