
#include "Instancia.h"

void ejecutarReemplazo(int posReemplazo) {

	if (memoriaLlena() == 1) {
		switch(configuracion.algoritmoDeReemplazo){

		case ALGORITMO_CIRC:

			reemplazoCircular(posReemplazo);

		break;

		case ALGORITMO_LRU:
			posReemplazo=reemplazoLRU();


		break;

		case ALGORITMO_BSU:
			posReemplazo=reemplazoBSU();


		break;

		}
	}

}
int cantElementosTabla(){

	return sizeof(tablaDeEntradas)/sizeof(tablaDeEntradas[0]);
}


int memoriaLlena(){
	int i;
	int cantEntradasOcupadas=0;
	int cantEntradas = cantElementosTabla;


	for(i=0;i<=cantEntradas;i++){
		if((tablaDeEntradas[i].ocupada)==1){
			cantEntradasOcupadas++;
		}
	}
	if(cantEntradasOcupadas==cantEntradas){
		return 1;
	}else return 0;

}

int reemplazoCircular(int posReemplazoActual){
	int posReemplazoNueva=0;
	int cantEntradas = cantElementosTabla();

	if (posReemplazoActual<=cantEntradas-1){
		posReemplazoNueva=posReemplazoActual;
		posReemplazoActual++;
	}else {
		posReemplazoActual=0;
		posReemplazoNueva=posReemplazoActual;
		posReemplazoActual++;
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

	for (i = 0; i < cantEntradas-1; i++) {
		t_entrada* entradaActual = malloc(sizeof(t_entrada));

		strcpy(entradaActual->clave,tablaDeEntradas[i].clave);
		entradaActual->tamanio=tablaDeEntradas[i].tamanio;
		entradaActual->ultimaInstruccion=tablaDeEntradas[i].ultimaInstruccion;

		if(entradaActual->ultimaInstruccion>=entradaReemplazo->ultimaInstruccion){
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
	for (i = 0; i < cantEntradas-1; i++) {
		if((tablaDeEntradas[i].tamanio)<= (configuracion.tamanioEntrada)){
			if(tablaDeEntradas[i].ultimaInstruccion>=entradaReemplazo->ultimaInstruccion){
				posReemplazo=i;
			}

		}

	}
	return posReemplazo;

}

