void consola() {
	while (1) {
		char *comando = readline("Ingrese un comando: ");

		if (strcmp(comando, "holis") == 0) {
			puts("hoooliss!");
		} else if (strcmp(comando, "chauchi") == 0) {
			puts("chauchiii!!");
		} else if (strcmp(comando, "exit") == 0) {
			free(comando);
			return;
		}
		free(comando);
	}
}
