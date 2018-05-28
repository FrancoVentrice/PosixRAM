#include "pantalla.h"

void centrarTexto(char *czBuf) {
	/* imprime un texto centrado en pantalla de 80 columnas y hace retorno de carro */
	int i;
	i = (80-strlen(czBuf))/2;
	while(i-- > 0) printf(" ");
	printf("%s\n",czBuf);
	fflush(stdout);
}

void limpiarPantalla() {
	/* limpia la pantalla y mueve el cursor a la posición 1;1 */
	printf(CLEAR_SCR RESET_POS);
	fflush(stdout);
}

void mostrarTextoXY(int row, int column, char *czBuf) {
	/* imprime un texto en la fila 'row' y columna 'column' y hace un retorno de carro */
	printf("\x1b[%d;%dH%s\n", row, column, czBuf);
	fflush(stdout);
}
