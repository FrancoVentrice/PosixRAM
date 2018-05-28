#include "libgral.h"

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
