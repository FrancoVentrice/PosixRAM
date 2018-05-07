#include "libgral.h"

// imprime un texto centrado en pantalla de 80 columnas y hace retorno de carro
void centrarTexto(char *czBuf) {
	int i;
	i = (80-strlen(czBuf))/2;
	while(i-- > 0) printf(" ");
	printf("%s\n",czBuf);
}

void limpiarPantalla() {
	//limpia la pantalla y mueve el cursor a la posición 1;1
	printf("\e[2J\e[1;1H");
	fflush(stdout);
}

void retardoSegundos(int iSegundos) {
	struct timeval tv;
	tv.tv_sec = iSegundos;
	tv.tv_usec = 0;
	select(1,NULL,NULL,NULL,&tv);
}

void retardoMilisegundos(int iMilisegundos) {
	//1 millisecond = 1000 microseconds
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = iMilisegundos * 1000;
	select(1,NULL,NULL,NULL,&tv);
}
