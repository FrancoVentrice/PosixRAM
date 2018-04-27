#include "libgral.h"

void centrarTexto(char *czBuf) {
	int i;
	i = (80-strlen(czBuf))/2;
	while(i-- > 0) printf(" ");
	printf("%s\n",czBuf);
}

void limpiarPantalla() {
	printf("\e[2J\e[1;1H");
	fflush(stdout);
}

void retardoSegundos(int iSegundos) {
	struct timeval tv;
	tv.tv_sec = iSegundos;
	tv.tv_usec = 0;
	select(1,NULL,NULL,NULL,&tv);
}

void capturaSenial(int iSignal) {
	switch(iSignal) {
		case SIGINT:
		break;
		case SIGTERM:
		break;
		case SIGCHLD:
			signal(SIGCHLD,SIG_IGN);
			while(waitpid(0,NULL,WNOHANG)>0)
			signal(SIGCHLD,capturaSenial);
		break;
	}
}
