#ifndef _LIBGRAL_H
#define _LIBGRAL_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#define STDIN 0

void centrarTexto(char *);
void limpiarPantalla();
void retardoSegundos(int iSegundos);
void capturaSenial(int iSignal);

#endif