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
		tablaDeEntradas[i].tamanio = (size_t) 0;
		tablaDeEntradas[i].ultimaInstruccion = 0;
	}
}

int entradasDisponibles() {
	/* Determina cuántas entradas quedan por ocupar usando como criterio que una entrada
	 * está libre si no tiene clave */

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
	/* Proceso de dump. Vuelca las entradas con sus valores en el punto de montaje */

	int i=0;

	log_info(logger,"Ejecutando proceso de vuelco...");
	mostrarTexto(AMARILLO_T "Ejecutando proceso de vuelco...");

	while (i < configuracion.cantidadEntradas) {
		if (!string_is_empty(tablaDeEntradas[i].clave)) {
			storeClave(i);
			i = i + entradasOcupadasPorClave(i);
		}
		else
			i++;
	}

	configuracion.ultimoDump = time(NULL);
	log_info(logger,"Vuelco finalizado.");
	retardoSegundos(2);
	refrescarPantalla();
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
		if (info.st_mode & S_IFDIR)
			log_info(logger,"Punto de montaje encontrado. Se deberán procesar las entradas existentes.");
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
			else
				log_info(logger,"El directorio se creó satisfactoriamente.");
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

	log_debug(logger,"Cargando archivos desde el punto de montaje");

	entradasACargar = string_split(clavesSincronizadas, ";");

    while (entradasACargar[j] != NULL) {

    	posicionValor = almacenamientoEntradas + (i * configuracion.tamanioEntrada);

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
				log_debug(logger,"Se leyeron %d bytes", (int)tablaDeEntradas[i].tamanio);
			}

			// unstable code: comparo un size_t con int pero... dale que va
			if (tablaDeEntradas[i].tamanio > configuracion.tamanioEntrada) {
				/* si el valor ocupa más de una entrada tengo que reflejarlo en la tabla */
				int entradasExtraOcupadas;
				entradasExtraOcupadas = entradasOcupadasPorValor(tablaDeEntradas[i].tamanio) - 1;
				while (entradasExtraOcupadas) {
					i++;
					strcpy(tablaDeEntradas[i].clave,tablaDeEntradas[i-1].clave);
					entradasExtraOcupadas--;
				}
			}
    	}
    	else {
    		log_warning(logger, "No existe el archivo: %s", archivoFullPath);
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

	log_debug(logger,"Procesar claves y cargar entradas");

	if (!string_is_empty(clavesSincronizadas)) {
		cargarEntradasDesdeArchivos(clavesSincronizadas);
	}

	free(clavesSincronizadas);
	return 1;
}

char * valorDeEntradaPorClave(char * clave) {
	/* Lee el valor correspondiente a una entrada de clave NNNNN, le agrega el terminador, y lo retorna */

	log_debug(logger,"Valor de la clave: %s", clave);

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

	log_debug(logger,"Valor para la entrada nro %d", indice);

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

	log_debug(logger,"Buscando índice de la clave %s", claveBuscada);

	while (encontrado < 0 && i < configuracion.cantidadEntradas) {
		if (strcmp(tablaDeEntradas[i].clave, claveBuscada) == 0)
			encontrado = i;
		else
			i++;
	}

	return encontrado;
}

bool existeClave(char * claveBuscada) {
	/* Devuelve TRUE si existe la clave en la tabla de entradas o FALSE si no existe */

	log_debug(logger,"Verificando existencia de clave %s", claveBuscada);

	return (indiceClave(claveBuscada) >= 0);
}

void storeClave(int indice) {
	/* Guarda el valor en el punto de montaje en un archivo de nombre clave.
	 * Si no tiene valor, guarda un archivo vacío.
	 * Si el archivo existe, lo sobreescribe. */

    char * archivoFullPath;
	int fdArchivoDestino;
	char * archivoMapeado;
	char * posicionValor;
	size_t tamanioValor;

	log_debug(logger,"Guardando clave y valor de entrada nro %d", indice);

	posicionValor = almacenamientoEntradas + (indice * configuracion.tamanioEntrada);
	tamanioValor = tablaDeEntradas[indice].tamanio;

	archivoFullPath = string_new();
	string_append_with_format(&archivoFullPath, "%s/%s", configuracion.puntoDeMontaje, tablaDeEntradas[indice].clave);

	log_info(logger, "Escribiendo archivo: %s", archivoFullPath);

	fdArchivoDestino = open(archivoFullPath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

	if(tamanioValor > 0) {
		lseek(fdArchivoDestino, tamanioValor-1, SEEK_SET);
		write(fdArchivoDestino, "", 1);

		archivoMapeado = (char *) mmap(0, tamanioValor, PROT_READ | PROT_WRITE, MAP_SHARED, fdArchivoDestino, 0);

		memcpy(archivoMapeado, posicionValor, tamanioValor);
		msync(archivoMapeado, tamanioValor, MS_SYNC);
		munmap(archivoMapeado, tamanioValor);

		log_debug(logger,"Se escribieron %d bytes", (int)tamanioValor);
	}
	close(fdArchivoDestino);
	free(archivoFullPath);

	tablaDeEntradas[indice].ultimaInstruccion = configuracion.instruccionesProcesadas;
}

int realizarCompactacion() {
	/* Compactación. Reacomoda la tabla de entradas y el espacio de almacenamiento, dejando
	 * el espacio libre al final.
	 * Retorna el índice de la primer entrada disponible. */

	int indice, proximaOcupada;
	char * posicionLibre = NULL;
	char * posicionValor = NULL;

	log_info(logger,"Ejecutando compactación...");
	mostrarTexto(AMARILLO_T "Ejecutando compactación...");

	log_debug(logger,"...entradas disponibles al inicio: %d", entradasDisponibles());

	for (indice=0 ; indice < configuracion.cantidadEntradas ; indice++) {
		if (string_is_empty(tablaDeEntradas[indice].clave)) {
			// se encontró entrada disponible
			proximaOcupada = indice + 1;
			while(string_is_empty(tablaDeEntradas[proximaOcupada].clave) && proximaOcupada < configuracion.cantidadEntradas)
				proximaOcupada++;

			if (proximaOcupada < configuracion.cantidadEntradas) {
				// actualizo tabla de entradas
				strcpy(tablaDeEntradas[indice].clave,tablaDeEntradas[proximaOcupada].clave);
				tablaDeEntradas[indice].tamanio = tablaDeEntradas[proximaOcupada].tamanio;
				tablaDeEntradas[indice].ultimaInstruccion = tablaDeEntradas[proximaOcupada].ultimaInstruccion;
				memset(tablaDeEntradas[proximaOcupada].clave, 0, MAX_LONG_CLAVE);
				tablaDeEntradas[proximaOcupada].tamanio = 0;
				tablaDeEntradas[proximaOcupada].ultimaInstruccion = 0;

				// muevo bloqueo de datos
				posicionLibre = almacenamientoEntradas + (indice * configuracion.tamanioEntrada);
				posicionValor = almacenamientoEntradas + (proximaOcupada * configuracion.tamanioEntrada);
				memcpy(posicionLibre, posicionValor, configuracion.tamanioEntrada);
				memset(posicionValor, 0, configuracion.tamanioEntrada);
			}
		}
	}

	indice = 0;
	while(!string_is_empty(tablaDeEntradas[indice].clave) && indice < configuracion.cantidadEntradas)
		indice++;

	log_info(logger,"Compactación finalizada. Primer entrada disponible: %d", indice);
	log_debug(logger,"...entradas disponibles al finalizar: %d", entradasDisponibles());
	retardoSegundos(2);
	refrescarPantalla();

	return indice;
}

int setClaveValor(char * claveRecibida, char * valorRecibido, t_respuestaSet * respuestaSet) {
	/* Realiza el set de la clave con el valor requerido. Aplica los algoritmos de reemplazo en caso de ser
	 * necesario, contempla si la entrada es nueva o no.
	 * Returno 1 si fue exitoso o 0 en caso de error (solo puede haber error si no pudo reemplazar por
	 * ser todas las entradas no atómicas) */

	int iEntradasDisponibles, iEntradasRequeridas, iEntradasOcupadas;
	int indiceOcupado, i;
	char * posicionValor;

	log_debug(logger,"SET de clave %s y valor %s", claveRecibida, valorRecibido);

	strcpy(respuestaSet->claveReemplazada,claveRecibida);
	respuestaSet->compactacionRequerida = 0;

	iEntradasDisponibles = entradasDisponibles();

	iEntradasRequeridas = entradasOcupadasPorValor(strlen(valorRecibido));
	log_debug(logger,"Entradas requeridas %d",iEntradasRequeridas);

	/* El algoritmo que sigue, lleno de IF, es lo más horrible de este TP.
	 * Chicos, no hagan esto en casa (pero funciona de 10 eh, eso no se discute) */

	if (existeClave(claveRecibida)) { // clave existente
		log_info(logger,"Realizando SET de clave existente: %s", claveRecibida);

		indiceOcupado = indiceClave(claveRecibida);
		iEntradasOcupadas = entradasOcupadasPorClave(indiceOcupado);
		log_debug(logger,"Entradas ocupadas %d",iEntradasOcupadas);

		if (iEntradasOcupadas == iEntradasRequeridas) {
			tablaDeEntradas[indiceOcupado].tamanio = strlen(valorRecibido);
			posicionValor = almacenamientoEntradas + (indiceOcupado * configuracion.tamanioEntrada);
			memcpy(posicionValor, valorRecibido, tablaDeEntradas[indiceOcupado].tamanio);
		}
		else if (iEntradasOcupadas > iEntradasRequeridas) {
			tablaDeEntradas[indiceOcupado].tamanio = strlen(valorRecibido);
			posicionValor = almacenamientoEntradas + (indiceOcupado * configuracion.tamanioEntrada);
			memcpy(posicionValor, valorRecibido, tablaDeEntradas[indiceOcupado].tamanio);

			for (i = iEntradasRequeridas; i < iEntradasOcupadas; i++) {
				memset(tablaDeEntradas[indiceOcupado+i].clave, 0, MAX_LONG_CLAVE);
				tablaDeEntradas[indiceOcupado+i].tamanio = (size_t) 0;
				tablaDeEntradas[indiceOcupado+i].ultimaInstruccion = 0;
			}
		}
		else { //iEntradasOcupadas < iEntradasRequeridas
			// todo parece que este caso no se va a probar (zafamos)
			if (iEntradasOcupadas + iEntradasDisponibles >= iEntradasRequeridas) {
				log_debug(logger,"No debería darse este caso!! (sobreescribir la entrada [1])");
				exit(1);
			}
			else {
				log_debug(logger,"No debería darse este caso!! (reemplazar [2])");
				exit(1);
			}
		}
	}
	else { // clave nueva
		log_info(logger, "Realizando SET de clave nueva: %s", claveRecibida);

		if (iEntradasDisponibles < iEntradasRequeridas) {
			if(!existenEntradasAtomicasParaReemplazar(iEntradasRequeridas - iEntradasDisponibles)) {
				log_error(logger, "No existen entradas atómicas suficientes para realizar el reemplazo.");
				return 0; // debería ser el único caso de error
			}
			ejecutarReemplazo(iEntradasRequeridas - iEntradasDisponibles);
			// todo actualizar en la respuesta el valor de la clave reemplazada
		}

		indiceOcupado = buscarEspacioContiguoDeEntradas(iEntradasRequeridas);
		if(indiceOcupado < 0) {// no se encontró espacio contiguo
			indiceOcupado = realizarCompactacion();
			respuestaSet->compactacionRequerida = 1;
		}

		strcpy(tablaDeEntradas[indiceOcupado].clave,claveRecibida);
		tablaDeEntradas[indiceOcupado].tamanio = strlen(valorRecibido);
		posicionValor = almacenamientoEntradas + (indiceOcupado * configuracion.tamanioEntrada);
		memcpy(posicionValor, valorRecibido, tablaDeEntradas[indiceOcupado].tamanio);
	}
	tablaDeEntradas[indiceOcupado].ultimaInstruccion = configuracion.instruccionesProcesadas;

	log_debug(logger, "SET realizado correctamente");
	return 1;
}


int buscarEspacioContiguoDeEntradas(int cantRequerida) {
	/* Busca una cantidad de entradas contiguas disponibles, y retorna el índice o -1 si no lo encuentra */

	int i, j;
	int encontrado = 0;

	log_debug(logger,"Buscando espacio contiguo para %d entradas", cantRequerida);

	i=0 ;
	while (i < configuracion.cantidadEntradas && !encontrado) {
		j = i;
		while (string_is_empty(tablaDeEntradas[j].clave) && j < configuracion.cantidadEntradas)
			j++;
		if ((j - i) >= cantRequerida)
			encontrado = 1;
		else
			i++;
	}

	if (encontrado)
		return i;
	else
		return -1;
}

bool esEntradaAtomica(int indice) {
	/* Indica si una entrada es atómica o no */

	log_debug(logger,"Verificando si la entrada nro %d tiene un valor atómico", indice);

	return (tablaDeEntradas[indice].tamanio <= configuracion.tamanioEntrada) &&
			(indice == 0 || (strcmp(tablaDeEntradas[indice].clave , tablaDeEntradas[indice-1].clave)));
}

int entradasOcupadasPorClave(int indice) {
	/* retorna la cantidad de entradas que ocupa el valor almacenado para una clave */

	log_debug(logger,"Calculando entradas ocupadas por clave nro %d", indice);

	if(esEntradaAtomica(indice))
		return 1;
	else
		return entradasOcupadasPorValor(tablaDeEntradas[indice].tamanio);
}

int entradasOcupadasPorValor(size_t tamanio) {
	/* retorna la cantidad de entradas requeridas para un cierto tamaño */

	int iEntradasOcupadas;

	log_debug(logger,"Calculando entradas ocupadas para valor de tamaño %d", (int)tamanio);

	iEntradasOcupadas = (int)(tamanio / configuracion.tamanioEntrada);
	if ((tamanio % configuracion.tamanioEntrada) != 0)
		iEntradasOcupadas++;

	return iEntradasOcupadas;
}
