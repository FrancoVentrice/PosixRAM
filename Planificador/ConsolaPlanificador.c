#include "Planificador.h"

void consola() {
	while (1) {
		char *comando = readline("Ingrese un comando: ");

		if (strcmp(comando, "pause") == 0) {
			pause();
		} else if (strcmp(comando, "play") == 0) {
			play();
		} else if (string_starts_with(comando, "lock")) {
			lock(comando);
		} else if (string_starts_with(comando, "unlock")) {
			unlock(comando);
		} else if (string_starts_with(comando, "list")) {
			list(comando);
		} else if (string_starts_with(comando, "kill")) {
			kill(comando);
		} else if (string_starts_with(comando, "status")) {
			status(comando);
		} else if (strcmp(comando, "deadlock") == 0) {
			deadlock();
		} else if (strcmp(comando, "exit") == 0) {
			free(comando);
			return;
		}
		free(comando);
	}
}

char* obtenerPrimerParametro(char *comando) {
	char** palabras = string_split(comando, " ");
	return palabras[1];
}

char* obtenerSegundoParametro(char *comando) {
	char** palabras = string_split(comando, " ");
	return palabras[2];
}

void pause() {
	puts("\nlogica para pausar planificador\n");
}

void play() {
	puts("\nlogica para continuar planificacion\n");
}

void lock(char *comando) {
	char *clave = obtenerPrimerParametro(comando);
	char *esi = obtenerSegundoParametro(comando);
	printf("logica para bloquear esi %s en cola de clave %s", esi, clave);
}

void unlock(char *comando) {
	char *esi = obtenerPrimerParametro(comando);
	printf("\nlogica para desbloquear proceso ESI %s\n", esi);
}

void list(char *comando) {
	char *recurso = obtenerPrimerParametro(comando);
	printf("\nlogica para listar procesos encolados esperando al recurso %s\n", recurso);
}

void kill(char *comando) {
	char *esi = obtenerPrimerParametro(comando);
	printf("\nlogica para matar proceso %s\n", esi);
}

void status(char *comando) {
	char *clave = obtenerPrimerParametro(comando);
	printf("\nlogica para mostrar estatus de clave %s\n", clave);
}

void deadlock() {
	puts("\nlogica para analizar deadlock\n");
}
