/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM
 *
 * Funciones útiles para que no se usan porque se cambiaron por otras
 *
 * */

#include "Instancia.h"

int deprecated_sincronizarClavesYCargarEntradas() {
	/* Le pide la lista de claves al coordinador y las carga en la tabla de entradas */

	char * clavesSincronizadas;

	clavesSincronizadas = deprecated_sincronizarClavesConCoordinador();
	if (string_is_empty(clavesSincronizadas)) // clavesSincronizadas == NULL  // strlen(text) == 0
		return 1;

	cargarEntradasDesdeArchivos(clavesSincronizadas);
	free(clavesSincronizadas);

	return 1;
}



char * deprecated_sincronizarClavesConCoordinador() {
	/* Le pide al coordinador la lista de claves, por si se está reconectando.
	 * No sale de la función hasta que recibe la respuesta, por lo que se podría quedar acá. */

	int bytesEnviados;

	log_info(logger,"Sincronizando claves con el Coordinador.");

	tPaquete pkgSincronizar;
	//pkgSincronizar.type = I_SINCRO_ENTRADAS;
	pkgSincronizar.type = I_RESULTADO_ERROR;
	pkgSincronizar.length = serializar(pkgSincronizar.payload, "", NULL);

	bytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgSincronizar, logger,"Enviando pedido de sincronizar claves...");
	log_debug(logger,"Se enviaron %d bytes", bytesEnviados);

	char * sPayloadRespuesta = (char *)malloc(configuracion.cantidadEntradas * MAX_LONG_CLAVE);
	memset(sPayloadRespuesta,0,configuracion.cantidadEntradas * MAX_LONG_CLAVE);
	char * mensajeRespuesta = (char *)malloc(configuracion.cantidadEntradas * MAX_LONG_CLAVE);
	memset(mensajeRespuesta,0,configuracion.cantidadEntradas * MAX_LONG_CLAVE);

	tMensaje tipoMensaje;
	recibirPaquete(configuracion.fdSocketCoordinador, &tipoMensaje, &sPayloadRespuesta, logger, "Recibiendo lista de claves...");
	//  El tipoMensaje es anecdótico en este punto, porque sabemos que va a ser C_SINCRO_ENTRADAS
	// log_debug(logger,"Mensaje recibido del coordinador tipo %d (Sincro entradas = %d)",tipoMensaje,C_SINCRO_ENTRADAS);
	deserializar(sPayloadRespuesta, "%s", mensajeRespuesta);
	free(sPayloadRespuesta);

	if(string_is_empty(mensajeRespuesta))
		log_info(logger,"No hay entradas para sincronizar.");
	else
		log_info(logger,"Listado de entradas recibido: %s", mensajeRespuesta);

	return mensajeRespuesta;
}


void deprecated_cargarEntradasDesdeArchivos() {
	/* lee los archivos del punto de montaje y carga las entradas con sus valores */

	unsigned int i = 0;
    char * archivoFullPath;
	int fdArchivoLeido;
	struct stat infoArchivo;
	char * archivoMapeado;
	char * posicionValor;

    struct dirent * archivoClave;
    DIR *dir = opendir(configuracion.puntoDeMontaje);

    if (dir == NULL) {
    	log_debug(logger,"Error leyendo el directorio. [%d - %s]", errno, strerror(errno));
        return;
    }

    while ((archivoClave = readdir(dir)) != NULL) {
    	if (!strcmp (archivoClave->d_name, "."))
    		continue;
    	if (!strcmp (archivoClave->d_name, ".."))
    		continue;

    	posicionValor = almacenamientoEntradas + (i * configuracion.tamanioEntrada);

    	log_info(logger, "Leyendo archivo: %s", archivoClave->d_name);
    	strcpy(tablaDeEntradas[i].clave,archivoClave->d_name);
    	archivoFullPath = string_new();
    	string_append_with_format(&archivoFullPath, "%s/%s", configuracion.puntoDeMontaje, archivoClave->d_name);
    	stat (archivoFullPath, &infoArchivo);
    	if (!(infoArchivo.st_size == 0)) {
    		tablaDeEntradas[i].tamanio = (size_t) infoArchivo.st_size;
        	fdArchivoLeido = open (archivoFullPath, O_RDONLY);
    		archivoMapeado = (char *) mmap(NULL, tablaDeEntradas[i].tamanio, PROT_READ, MAP_SHARED, fdArchivoLeido, 0);
        	memcpy(posicionValor, archivoMapeado, tablaDeEntradas[i].tamanio);
        	munmap(archivoMapeado, tablaDeEntradas[i].tamanio);
    		close(fdArchivoLeido);
    	}
    	free(archivoFullPath);

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
    	i++;
    }

    closedir(dir);
    log_info(logger,"Se cargaron %d entradas.", i);
}


void deprecated_ejecutarSet(char *clave, char *valor, int posicion) {
	char * posicionValor;
	unsigned int i = posicion;

	posicionValor = almacenamientoEntradas + (i * configuracion.tamanioEntrada);
	//ToDo revisar ===> posicionValor = almacenamientoEntradas[i * configuracion.tamanioEntrada];

	strcpy(tablaDeEntradas[i].clave, clave);
	tablaDeEntradas[i].tamanio = string_length(valor);
	memcpy(posicionValor, valor, tablaDeEntradas[i].tamanio);

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


void deprecated_enviarMensajeOK() {

	tPaquete pkgSetOk;
	int bytesEnviados;
	char* lineaOk = malloc(5);
	strcpy(lineaOk, "OK");

	pkgSetOk.type = I_RESULTADO_SET;
	pkgSetOk.length = serializar(pkgSetOk.payload, "%s", lineaOk);

	log_debug(logger, "Se envia %s al Planificador", lineaOk);
	bytesEnviados = enviarPaquete(configuracion.fdSocketCoordinador, &pkgSetOk, logger, "Se envia OK al Planificador");
	log_debug(logger, "Se envian %d bytes", bytesEnviados);
	free(lineaOk);
}
