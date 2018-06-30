/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Instancia.h"

void prepararTablaDeEntradas() {
	/* prepara la estructura de la tabla de entradas y asigna el espacio para almacenar los valores */

	unsigned int espacioTotal;
	int i;

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

int entradasDisponibles() {
	/* determina cuántas entradas quedan por ocupar */

	int entradasDisponibles;
	int i;

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

void volcarEntradasEnArchivos() {
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

void cargarEntradasDesdeArchivos(char * clavesSincronizadas) {
	/* lee los archivos del punto de montaje y carga las entradas con sus valores.
	 * recibe un string que es una lista de entradas separadas por ; */

	int i = 0, j = 0;
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

			// unstable code: comparo un size_t con int pero... dale que va
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

char * valorDeEntradaPorClave(char * clave) {
	/* Lee el valor correspondiente a una entrada de clave NNNNN, le agrega el terminador, y lo retorna */

	int indice;
	indice = indiceClave(clave);

	if (indice < 0)
		return strdup("");
	else
		return valorDeEntradaPorIndice(indice);
}

char * valorDeEntradaPorIndice(int indice) {
	/* Lee el valor correspondiente a una entrada i, le agrega el terminador, y lo retorna */

	char * valor;
	valor = (char *)malloc((tablaDeEntradas[indice].tamanio)+1);
	memcpy(valor,almacenamientoEntradas + (indice * configuracion.tamanioEntrada), tablaDeEntradas[indice].tamanio);
	valor[tablaDeEntradas[indice].tamanio] = '\0';

	// se debe hacer el free del valor devuelvo en el lugar que se utilice
	return valor;
}

int indiceClave(char * claveBuscada) {
	/* Devuelve el índice de la clave en la tabla de entradas, o -1 si no existe */

	int i = 0;
	int encontrado = -1;

	while (encontrado < 0 && i < configuracion.cantidadEntradas) {
		if (strcmp(tablaDeEntradas[i].clave, claveBuscada) == 0)
			encontrado = i;
		else
			i++;
	}

	return encontrado;
}

int existeClave(char * claveBuscada) {
	/* Devuelve 1 si existe la clave en la tabla de entradas o 0 si no existe */

	if(indiceClave(claveBuscada) < 0)
		return 0;
	else
		return 1;
}

void storeClave(char * unaClave) {
	/* Guarda el valor en el punto de montaje en un archivo de nombre clave.
	 * Si no tiene valor, guarda un archivo vacío.
	 * Si el archivo existe, lo sobreescribe. */

    char * archivoFullPath;
	int fdArchivoDestino;
	char * archivoMapeado;
	int indice;
	char * posicionValor;
	size_t tamanioValor;

	indice = indiceClave(unaClave);
	posicionValor = almacenamientoEntradas + (indice * configuracion.tamanioEntrada);
	tamanioValor = tablaDeEntradas[indice].tamanio;

	archivoFullPath = string_new();
	string_append_with_format(&archivoFullPath, "%s/%s", configuracion.puntoDeMontaje, unaClave);

	log_info(logger, "Escribiendo archivo: %s", archivoFullPath);

	fdArchivoDestino = open(archivoFullPath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

	if(tamanioValor > 0) {
		lseek(fdArchivoDestino, tamanioValor-1, SEEK_SET);
		write(fdArchivoDestino, "", 1);

		archivoMapeado = (char *) mmap(0, tamanioValor, PROT_READ | PROT_WRITE, MAP_SHARED, fdArchivoDestino, 0);

		memcpy(archivoMapeado, posicionValor, tamanioValor);
		msync(archivoMapeado, tamanioValor, MS_SYNC);
		munmap(archivoMapeado, tamanioValor);
	}
	close(fdArchivoDestino);
	free(archivoFullPath);

	tablaDeEntradas[indice].ultimaInstruccion = configuracion.instruccionesProcesadas;
}
