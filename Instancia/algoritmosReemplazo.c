
#include "Instancia.h"

void ejecutarReemplazo(int *posReemplazo, int *posActualCircular, t_entrada* entrada) {

	if (memoriaLlena() == 1) {
		switch(configuracion.algoritmoDeReemplazo){

		case ALGORITMO_CIRC:

			*posReemplazo=reemplazoCircular(posActualCircular);

		break;

		case ALGORITMO_LRU:
			*posReemplazo=reemplazoLRU();


		break;

		case ALGORITMO_BSU:
			*posReemplazo=reemplazoBSU();


		break;

		}
		realizarReemplazo(*posReemplazo,entrada);

	}

}

void realizarReemplazo(int posReemplazo, t_entrada* entrada){

	tablaDeEntradas[posReemplazo].clave=entrada->clave;
	tablaDeEntradas[posReemplazo].ocupada=1;
	tablaDeEntradas[posReemplazo].tamanio=entrada->tamanio;
	tablaDeEntradas[posReemplazo].ultimaInstruccion=entrada->ultimaInstruccion;

}

int cantElementosTabla(){

	return sizeof(tablaDeEntradas)-1;

}

int memoriaLlena(){
	int i;
	int cantEntradasOcupadas=0;
	int cantEntradas = cantElementosTabla();


	for(i=0;i<=cantEntradas;i++){
		if((tablaDeEntradas[i].ocupada)==1){
			cantEntradasOcupadas++;
		}
	}
	if(cantEntradasOcupadas==cantEntradas){
		return 1;
	}else return 0;

}

int reemplazoCircular(int *posActual){
	int posReemplazoNueva=0;
	int cantEntradas = cantElementosTabla();

	if (*posActual<=cantEntradas-1){
		posReemplazoNueva=*posActual;
		(*posActual)++;
		return posReemplazoNueva;
	}else {
		posReemplazoNueva=0;
		*posActual=posReemplazoNueva+1;
		return posReemplazoNueva;

	}
return posReemplazoNueva;
}


int reemplazoLRU() {

	int i;
	int posReemplazo = 0;
	int cantEntradas = cantElementosTabla();
	t_entrada* entradaReemplazo = malloc(sizeof(t_entrada));
	strcpy(entradaReemplazo->clave,tablaDeEntradas[0].clave);
	entradaReemplazo->tamanio=tablaDeEntradas[0].tamanio;
	entradaReemplazo->ultimaInstruccion=tablaDeEntradas[0].ultimaInstruccion;

	for (i = 0; i <=cantEntradas-1; i++) {
		t_entrada* entradaActual = malloc(sizeof(t_entrada));

		strcpy(entradaActual->clave,tablaDeEntradas[i].clave);
		entradaActual->tamanio=tablaDeEntradas[i].tamanio;
		entradaActual->ultimaInstruccion=tablaDeEntradas[i].ultimaInstruccion;

		if(entradaActual->ultimaInstruccion<entradaReemplazo->ultimaInstruccion){
			entradaReemplazo=entradaActual;
			posReemplazo=i;
		}

	}

	return posReemplazo;

}

int reemplazoBSU(){
	int i;
	int posReemplazo = 0;
	int cantEntradas = cantElementosTabla();
	t_entrada* entradaReemplazo =malloc(sizeof(t_entrada));
	strcpy(entradaReemplazo->clave,tablaDeEntradas[0].clave);
	entradaReemplazo->tamanio=tablaDeEntradas[0].tamanio;
	entradaReemplazo->ultimaInstruccion=tablaDeEntradas[0].ultimaInstruccion;
	for (i = 0; i <=cantEntradas-1; i++) {
		if((tablaDeEntradas[i].tamanio)<= (configuracion.tamanioEntrada)){
			if(tablaDeEntradas[i].ultimaInstruccion>=entradaReemplazo->ultimaInstruccion){
				posReemplazo=i;
			}

		}

	}
	return posReemplazo;

}

