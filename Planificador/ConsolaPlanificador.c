#include "Planificador.h"

void consola() {
	printf("\n\n\nBienvenido al Planificador PosixRAM para ReDistinto\n\n\n");
	printf("Comandos disponibles:\n");
	printf("-  pause  (pausa ejecucion, respeta atomicidad de las operaciones)\n");
	printf("-  play  (reanuda ejecucion)\n");
	printf("-  lock <clave> <esi>  (bloquea al ESI <esi> en la cola del recurso <clave>)\n");
	printf("-  unlock <clave>  (libera al primer ESI de la cola del recurso <clave>. Si la cola queda vacia, la clave es liberada)\n");
	printf("-  list <clave>  (lista los ESIs encolados esperando al recurso <clave>)\n");
	printf("-  kill <esi>  (aborta al ESI <esi>, respetando atomicidad y liberando las claves que tenga tomadas)\n");
	printf("-  status <clave>  (lista el valor, instancia que lo aloja, y ESIs encolados esperando al recurso <clave>)\n");
	printf("-  deadlock  (lista ESIs implicados en un deadlock)\n");
	printf("-  exit  (terminacion del proceso Planificador)\n\n\n");
	while (1) {
		char *comando = readline("Ingrese un comando: ");
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
			list(comando);
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
		log_error(logger, "No hay suficientes parametros\n");
		return NULL;
	}
	char** palabras = string_split(comando, " ");
	return palabras[1];
}

char* obtenerSegundoParametro(char *comando) {
	char** palabras = string_split(comando, " ");
	if (palabras[2] == NULL) {
		log_error(logger, "No hay suficientes parametros\n");
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
	if (clave == NULL) {
		return;
	}
	char *esi = obtenerSegundoParametro(comando);
	if (esi == NULL) {
		return;
	}
	newInstruccion(INSTRUCCION_BLOQUEAR, clave, esi);
}

void unlock(char *comando) {
	char *clave = obtenerPrimerParametro(comando);
	if (clave == NULL) {
		return;
	}
	newInstruccion(INSTRUCCION_DESBLOQUEAR, clave, NULL);
}

void list(char *comando) {
	char *recurso = obtenerPrimerParametro(comando);
	if (recurso == NULL) {
		return;
	}
	newInstruccion(INSTRUCCION_LISTAR, recurso, NULL);
}

void killEsi(char *comando) {
	char *esi = obtenerPrimerParametro(comando);
	if (esi == NULL) {
		return;
	}
	newInstruccion(INSTRUCCION_TERMINAR, esi, NULL);
}

void status(char *comando) {
	char *clave = obtenerPrimerParametro(comando);
	if (clave == NULL) {
		return;
	}
	newInstruccion(INSTRUCCION_STATUS, clave, NULL);
}

void deadlock() {
	newInstruccion(INSTRUCCION_DEADLOCK, NULL, NULL);
}

void exitPlanificador() {
	vivo = false;
	pthread_mutex_unlock(&mutexEspera);
	log_info(logger, "mutex unlock en exit");
}

void instruccionDestroyer(t_instruccion_consola *instruccion) {
	if (instruccion->primerParametro != NULL) {
		free(instruccion->primerParametro);
	}
	if (instruccion->segundoParametro != NULL) {
		free(instruccion->segundoParametro);
	}
	if (instruccion != NULL) {
		free(instruccion);
	}
}

void newInstruccion(int instruccion, char *primerParametro, char *segundoParametro) {
	t_instruccion_consola *instruccionConsola = malloc(sizeof(t_instruccion_consola));
	instruccionConsola->instruccion = instruccion;
	instruccionConsola->primerParametro = primerParametro;
	instruccionConsola->segundoParametro = segundoParametro;
	list_add(bufferConsola, instruccionConsola);
	pthread_mutex_unlock(&mutexEspera);
	log_info(logger, "mutex unlock en new instruccion");
}
