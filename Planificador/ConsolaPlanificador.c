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
	ejecutando = false;
}

void play() {
	ejecutando = true;
}

void lock(char *comando) {
	char *clave = obtenerPrimerParametro(comando);
	char *esi = obtenerSegundoParametro(comando);
	newInstruccion(INSTRUCCION_BLOQUEAR, clave, esi);
}

void unlock(char *comando) {
	char *clave = obtenerPrimerParametro(comando);
	newInstruccion(INSTRUCCION_DESBLOQUEAR, clave, NULL);
}

void list(char *comando) {
	char *recurso = obtenerPrimerParametro(comando);
	printf("\nlogica para listar procesos encolados esperando al recurso %s\n", recurso);
}

void kill(char *comando) {
	char *esi = obtenerPrimerParametro(comando);
	newInstruccion(INSTRUCCION_TERMINAR, esi, NULL);
}

void status(char *comando) {
	char *clave = obtenerPrimerParametro(comando);
	printf("\nlogica para mostrar estatus de clave %s\n", clave);
}

void deadlock() {
	newInstruccion(INSTRUCCION_DEADLOCK, NULL, NULL);
}

void exitPlanificador() {
	vivo = false;
}

void instruccionDestroyer(t_instruccion_consola *instruccion) {
	free(instruccion->primerParametro);
	free(instruccion->segundoParametro);
	free(instruccion);
}

void newInstruccion(int instruccion, char *primerParametro, char *segundoParametro) {
	t_instruccion_consola *instruccionConsola = malloc(sizeof(t_instruccion_consola));
	if (primerParametro) {
		instruccionConsola->primerParametro = primerParametro;
	}
	if (segundoParametro) {
		instruccionConsola->segundoParametro = segundoParametro;
	}
	list_add(bufferConsola, instruccion);
}
