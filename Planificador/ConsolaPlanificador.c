#include "Planificador.h"

void consola() {
	while (1) {
		char *comando = readline("Ingrese un comando: ");
		bool valido;
		if (strcmp(comando, "pause") == 0) {
			printf("Planificador pausado\n");
			pause();
		} else if (strcmp(comando, "play") == 0) {
			printf("Planificador reanudado\n");
			play();
		} else if (string_starts_with(comando, "lock")) {
			lock(comando);
		} else if (string_starts_with(comando, "unlock")) {
			unlock(comando);
		} else if (string_starts_with(comando, "list")) {
			valido = list(comando);
			if (valido) {
				free(comando);
				return;
			}
		} else if (string_starts_with(comando, "kill")) {
			killEsi(comando);
		} else if (string_starts_with(comando, "status")) {
			status(comando);
		} else if (strcmp(comando, "deadlock") == 0) {
			deadlock();
		} else if (strcmp(comando, "exit") == 0) {
			free(comando);
			exitPlanificador();
			return;
		}
		free(comando);
	}
}

char* obtenerPrimerParametro(char *comando) {
	if (!string_contains(comando, " ")) {
		printf("No hay suficientes parametros\n");
		return NULL;
	}
	char** palabras = string_split(comando, " ");
	return palabras[1];
}

char* obtenerSegundoParametro(char *comando) {
	char** palabras = string_split(comando, " ");
	if (palabras[2] == NULL) {
		printf("No hay suficientes parametros\n");
		return NULL;
	}
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

bool list(char *comando) {
	char *recurso = obtenerPrimerParametro(comando);
	if (recurso == NULL) {
		return false;
	}
	newInstruccion(INSTRUCCION_LISTAR, recurso, NULL);
	return true;
}

void killEsi(char *comando) {
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
	instruccionConsola->instruccion = instruccion;
	if (primerParametro) {
		instruccionConsola->primerParametro = primerParametro;
	}
	if (segundoParametro) {
		instruccionConsola->segundoParametro = segundoParametro;
	}
	list_add(bufferConsola, instruccionConsola);
}
