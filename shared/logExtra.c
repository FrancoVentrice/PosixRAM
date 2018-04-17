#include "logExtra.h"

/**
 * @NAME: logInit
 * @DESC: Arma el archivo de log con opciones copadas y lo devuelve
 * Primer parámetro : es un vector de argumentos, recibe  un parámetro por consola -v para verbose,
 * y dsps -ll para level trace, y -log path
 * para guardar el log en el path que se pase
 * Segundo parámetro: nombre de que proceso iniciaría el log.
 *
 *
 * 	Default:
 * 	No verbose
 * 	Nivel: Error
 * 	Path: /NOMBREPROCESO.log
*/

t_log* logInit(char** argv, char* who){
	int i = 0;
	//Valores default
	_Bool verbose = false;
	int logLevel = LOG_LEVEL_DEBUG;
	char path[30] = { 0 };
	strcpy(path, strcat(who,".log"));

	// Bucle que chequea argumentos
	while(argv[i] != NULL){
		if(string_equals_ignore_case(argv[i], "-v"))
			verbose = true;
		else if(string_equals_ignore_case(argv[i], "-ll")){ //Log level
			if(string_equals_ignore_case(argv[i+1], "trace"))
				logLevel = LOG_LEVEL_TRACE;
			if(string_equals_ignore_case(argv[i+1], "debug"))
				logLevel = LOG_LEVEL_DEBUG;
			if(string_equals_ignore_case(argv[i+1], "info"))
				logLevel = LOG_LEVEL_INFO;
			if(string_equals_ignore_case(argv[i+1], "warning"))
				logLevel = LOG_LEVEL_WARNING;
			if(string_equals_ignore_case(argv[i+1], "error"))
				logLevel = LOG_LEVEL_ERROR;
		} else if(string_equals_ignore_case(argv[i], "-log")){//path pasado a  mano
			strcpy(path, argv[i+1]);
		}
		i++;
	}// fin bucle

	char aux[40] = { 0 };
		strcat(aux, "touch ");
		strcat(aux, path);
	system(aux); // si el path no existe hace un touch para crearlo

	t_log* auxLog = log_create(path, who, verbose, logLevel); //crea el log

	log_info(auxLog, "--------------- Started log tracking ---------------");
	return auxLog;
}
