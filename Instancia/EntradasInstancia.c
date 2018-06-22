/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

void prepararTablaDeEntradas() {
	/* prepara la estructura de la tabla de entradas y asigna el espacio para almacenar los valores */

	unsigned int espacioTotal;
	unsigned int i;

	espacioTotal = (configuracion.cantidadEntradas) * (configuracion.tamanioEntrada);
	log_info(logger,"Reservando espacio de almacenamiento para %d bytes.",espacioTotal);
	almacenamientoEntradas = (char *)malloc(espacioTotal);
	memset(almacenamientoEntradas, 0, espacioTotal);

	log_info(logger,"Preparando tabla para %d entradas.",configuracion.cantidadEntradas);
	tablaDeEntradas = (t_entrada *)malloc((configuracion.cantidadEntradas) * sizeof(t_entrada));

	for (i=0 ; i < configuracion.cantidadEntradas ; i++) {
		log_debug(logger,"... seteando entrada %d",i);
		memset(tablaDeEntradas[i].clave, 0, MAX_LONG_CLAVE);
		tablaDeEntradas[i].tamanio = 0;
		tablaDeEntradas[i].ultimaInstruccion = 0;
	}
}

unsigned int entradasDisponibles() {
	/* determina cuántas entradas quedan por ocupar */

	unsigned int entradasDisponibles;
	unsigned int i;

	log_debug(logger,"Calculando entradas disponibles");
	entradasDisponibles = 0;

	for (i=0 ; i < configuracion.cantidadEntradas ; i++) {
		if (string_is_empty(tablaDeEntradas[i].clave))
				entradasDisponibles++;
	}

	return entradasDisponibles;
}

void iniciarDumpTimeout() {
	/* timeout para disparar señal de dump */

	configuracion.fdTimerDump = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    //fcntl(configuracion.fdTimerDump, F_SETFL, O_NONBLOCK);
    timerfd_settime(configuracion.fdTimerDump, 0, &(configuracion.intervaloDump), NULL);

	log_info(logger,"Seteada alarma para vuelco en %d segundos.", (int)configuracion.intervaloDump.it_value.tv_sec);
}

void volcarEntradas() {
	/* Proceso de dump. Vuelca las entradas con sus valores en el punto de montaje.
	 * Supuesto: no hay "basura" en el punto de montaje. */

	// TODO completar este proceso
	log_info(logger,"Ejecutando proceso de vuelco...");
	retardoSegundos(2);
	configuracion.ultimoDump = time(NULL);
	log_info(logger,"Vuelco finalizado.");
}

void limpiarTablaDeEntradas() {
	/* libera la entrada de entradas y el espacio de almacenamiento reservado */

	if (almacenamientoEntradas != NULL) {
		free(almacenamientoEntradas);
		free(tablaDeEntradas);
	}
}

int inicializarPuntoDeMontaje() {
	/* prepara el directorio del punto de montaje. */

	/* Unstable Code: no funciona la creación de la carpeta si faltan crear más carpetas del path */

	int e;
	struct stat info;

	log_info(logger,"Preparando punto de montaje.");
	e = stat(configuracion.puntoDeMontaje, &info);

	if (e == 0) {
		if (info.st_mode & S_IFREG) {
			log_error(logger,"El punto de montaje es un archivo.");
			mostrarTexto("ERROR: El punto de montaje es un archivo.");
			return 0;
		}
		if (info.st_mode & S_IFDIR) {
			log_info(logger,"Punto de montaje encontrado. Se deberán procesar las entradas existentes.");
			// cargarEntradasDesdeArchivos(); se desactiva porque se debe sincronizar con el coordinador
		}
	}
	else {
		if (errno == ENOENT) {
			log_warning(logger,"El punto de montaje no existe. Se creará el directorio.");
			e = mkdir(configuracion.puntoDeMontaje, ACCESSPERMS | S_IRWXU);
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

char * valorDeEntradaPorClave(char * clave) {
	/* Lee el valor correspondiente a una entrada de clave NNNNN, le agrega el terminador, y lo retorna */

	unsigned int i = 0;
	int encontrado = 0;

	while (!encontrado && i < configuracion.cantidadEntradas) {
		if (strcmp(tablaDeEntradas[i].clave, clave) == 0)
			encontrado = 1;
		else
			i++;
	}

	// ToDo si no la encontró hay que devolver error!
	return valorDeEntradaPorIndice(i);
}

char * valorDeEntradaPorIndice(unsigned int indice) {
	/* Lee el valor correspondiente a una entrada i, le agrega el terminador, y lo retorna */

	char * valor;
	valor = (char *)malloc((tablaDeEntradas[indice].tamanio)+1);
	memcpy(valor,almacenamientoEntradas + (indice * configuracion.tamanioEntrada), tablaDeEntradas[indice].tamanio);
	valor[tablaDeEntradas[indice].tamanio] = '\0';

	// se debe hacer el free del valor devuelvo en el lugar que se utilice
	return valor;
}

void cargarEntradasDesdeArchivos(char * clavesSincronizadas) {
	/* lee los archivos del punto de montaje y carga las entradas con sus valores.
	 * recibe un string que es una lista de entradas separadas por ; */

	unsigned int i = 0, j = 0;
	char ** entradasACargar;
    char * archivoFullPath;
	int fdArchivoLeido;
	struct stat infoArchivo;
	char * archivoMapeado;
	char * posicionValor;

	entradasACargar = string_split(clavesSincronizadas, ";");

    while (entradasACargar[j] != NULL) {

    	posicionValor = almacenamientoEntradas + (i * configuracion.tamanioEntrada);
    	// ToDo revisar esto --->> posicionValor = almacenamientoEntradas[i * configuracion.tamanioEntrada];

    	log_info(logger, "Cargando clave: %s", entradasACargar[j]);
    	strcpy(tablaDeEntradas[i].clave,entradasACargar[j]);
    	archivoFullPath = string_new();
    	string_append_with_format(&archivoFullPath, "%s/%s", configuracion.puntoDeMontaje, entradasACargar[j]);

    	if (stat(archivoFullPath, &infoArchivo) == 0) {
    		log_info(logger, "Leyendo archivo: %s", archivoFullPath);
			if (!(infoArchivo.st_size == 0)) {
				tablaDeEntradas[i].tamanio = (size_t) infoArchivo.st_size;
				fdArchivoLeido = open (archivoFullPath, O_RDONLY);
				archivoMapeado = (char *) mmap(NULL, tablaDeEntradas[i].tamanio, PROT_READ, MAP_SHARED, fdArchivoLeido, 0);
				memcpy(posicionValor, archivoMapeado, tablaDeEntradas[i].tamanio);
				munmap(archivoMapeado, tablaDeEntradas[i].tamanio);
				close(fdArchivoLeido);
			}

			// unstable code: comparo un size_t con unsigned int pero... dale que va
			if (tablaDeEntradas[i].tamanio > configuracion.tamanioEntrada) {
				/* si el valor ocupa más de una entrada tengo que reflejarlo en la tabla */
				int entradasExtraOcupadas;
				entradasExtraOcupadas = tablaDeEntradas[i].tamanio / configuracion.tamanioEntrada;
				while (entradasExtraOcupadas) {
					i++;
					strcpy(tablaDeEntradas[i].clave,tablaDeEntradas[i-1].clave);
					entradasExtraOcupadas--;
				}
			}
    	}
    	else {
    		log_info(logger, "No existe el archivo: %s", archivoFullPath);
    	}
		free(archivoFullPath);
    	i++;
    	free(entradasACargar[j]);
    	j++;
    }

    log_info(logger,"Se cargaron %d entradas desde %d claves.", i, j);
}

int procesarClavesYCargarEntradas(char * clavesSincronizadas) {
	/* Carga en la tabla de entradas con la lista de claves que recibió del coordinador en el handshake */

	if (!string_is_empty(clavesSincronizadas)) {
		cargarEntradasDesdeArchivos(clavesSincronizadas);
	}

	free(clavesSincronizadas);
	return 1;
}
