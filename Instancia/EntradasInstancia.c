/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

void prepararTablaDeEntradas() {
	/* prepara la estructura de la tabla de entradas y asigna el espacio para almacenar los valores */

	unsigned int espacioTotal;
	int i;

	espacioTotal = (configuracion->cantidadEntradas) * (configuracion->tamanioEntrada);
	log_info(logger,"Reservando espacio de almacenamiento para %d bytes.",espacioTotal);
	almacenamientoEntradas = (char *)malloc(espacioTotal);
	memset(almacenamientoEntradas, 0, espacioTotal);

	log_info(logger,"Preparando tabla para %d entradas.",configuracion->cantidadEntradas);
	tablaDeEntradas = (t_entrada *)malloc((configuracion->cantidadEntradas) * sizeof(t_entrada));

	for (i=0 ; i < configuracion->cantidadEntradas ; i++) {
		log_debug(logger,"... seteando entrada %d",i);
		memset(tablaDeEntradas[i].clave, 0, MAX_LONG_CLAVE);
		tablaDeEntradas[i].tamanio = 0;
		tablaDeEntradas[i].ultimaInstruccion = 0;
	}
}

unsigned int entradasDisponibles() {
	/* determina cuántas entradas quedan por ocupar */

	unsigned int entradasDisponibles;
	int i;

	log_debug(logger,"Calculando entradas disponibles");
	entradasDisponibles = 0;

	for (i=0 ; i < configuracion->cantidadEntradas ; i++) {
		if (string_is_empty(tablaDeEntradas[i].clave))
				entradasDisponibles++;
	}

	return entradasDisponibles;
}

void iniciarDumpTimeout() {
	/* timeout para disparar señal de dump */

	estadoInstancia.realizarDump = 0;
	signal(SIGALRM, capturaSenial);
	alarm(configuracion->intervaloDump);
	log_info(logger,"Seteada alarma para vuelco en %d segundos.",configuracion->intervaloDump);
}

void volcarEntradas() {
	/* proceso de dump */

	// TODO completar este proceso
	log_info(logger,"Ejecutando proceso de vuelco...");
	retardoSegundos(3);
	estadoInstancia.ultimoDump = time(NULL);
	log_info(logger,"Vuelco finalizado.");
	iniciarDumpTimeout();
}

void limpiarTablaDeEntradas() {
	/* libera la entrada de entradas y el espacio de almacenamiento reservado */

	if (almacenamientoEntradas != NULL) {
		free(almacenamientoEntradas);
		free(tablaDeEntradas);
	}
}

int inicializarPuntoDeMontaje() {
	/* prepara el directorio del punto de montaje. si existen archivos los levanta
	 * y registra las entradas correspondientes */

	/* Unstable Code: no funciona la creación de la carpeta si faltan crear más carpetas del path */

	int e;
	struct stat info;

	log_info(logger,"Preparando punto de montaje.");
	e = stat(configuracion->puntoDeMontaje, &info);

	if (e == 0) {
		if (info.st_mode & S_IFREG) {
			log_error(logger,"El punto de montaje es un archivo.");
			mostrarTexto("ERROR: El punto de montaje es un archivo.");
			return 0;
		}
		if (info.st_mode & S_IFDIR) {
			log_info(logger,"Punto de montaje encontrado. Se procesarán las entradas existentes.");
			cargarEntradasDesdeArchivos();
		}
	}
	else {
		if (errno == ENOENT) {
			log_warning(logger,"El punto de montaje no existe. Se creará el directorio.");
			e = mkdir(configuracion->puntoDeMontaje, ACCESSPERMS | S_IRWXU);
			if (e != 0) {
				log_error(logger,"Se produjo un error al crear el directorio. [%d - %s]", errno, strerror(errno));
				mostrarTexto("ERROR: Se produjo un error al crear el directorio.");
				return 0;
			}
			else {
				log_info(logger,"El directorio se creó satisfactoriamente.");
			}
		}
		else {
			log_error(logger,"Se produjo un error accediendo al punto de montaje. [%d - %s]", errno, strerror(errno));
			mostrarTexto("ERROR: Se produjo un error accediendo al punto de montaje.");
			return 0;
		}
	}
	return 1;
}

void cargarEntradasDesdeArchivos() {
	/* lee los archivos del punto de montaje y carga las entradas con sus valores */

    struct dirent *entrada;
    DIR *dir = opendir(configuracion->puntoDeMontaje);

    if (dir == NULL) {
        return;
    }

    // TODO corregir esta parte
    while ((entrada = readdir(dir)) != NULL) {
    	if (!strcmp (entrada->d_name, "."))
    		continue;
    	if (!strcmp (entrada->d_name, ".."))
    		continue;
        printf("%s\n",entrada->d_name);
    }

    closedir(dir);
}