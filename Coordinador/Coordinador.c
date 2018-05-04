/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "Coordinador.h"
#include "..//shared/sockets.h"
#include "..//shared/serializar.h"
#include "..//shared/protocolo.h"

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	escucharConexiones();
}
void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}

void escucharConexiones() {

	int puertoEscucha = configuracion->puertoEscucha;

	t_log *logCoordinador = log_create("coordinador.log", "COORDINADOR",
	true, LOG_LEVEL_INFO);
	int maxSock;
	int iSocketEscucha;
	int iSocketComunicacion;
	int tamanioTotal = 0;


	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log
	iSocketEscucha = crearSocketEscucha(puertoEscucha, logCoordinador);

	FD_SET(iSocketEscucha, &setSocketsOrquestador);
	maxSock = iSocketEscucha;

	tPaquete pkgHandshake;

	tMensaje *tipoMensaje = malloc(sizeof(tMensaje));
	char * sPayloadRespuesta = malloc(100);
	char * respuesta = malloc(100);

	char encabezadoMensaje;
	tSolicitudESI *solicitud = malloc(sizeof(tSolicitudESI));
	solicitud->mensaje = malloc(100);

	tSolicitudESI* solicitudESI = malloc(sizeof(tSolicitudESI));
	int recibidos;
	puts("Escuchando");

	while (1) {
		iSocketComunicacion = getConnection(&setSocketsOrquestador, &maxSock,
				iSocketEscucha, tipoMensaje, &sPayloadRespuesta,
				logCoordinador);

		if (iSocketComunicacion != -1) {

			switch (*tipoMensaje) {

			case E_HANDSHAKE:
				printf("Socket comunicacion: %d \n", iSocketComunicacion);

				puts("HANDSHAKE CON ESI");
				tSolicitudESI *mensaje = malloc(100);
				char* encabezado = malloc(10);

				deserializar(sPayloadRespuesta, "%c%s", encabezado, solicitud->mensaje);
				printf("MENSAJE DE ESI: %s\n", solicitud->mensaje);

				//RESPUESTA AL ESI
				tSolicitudESI* solicitudESI = malloc(sizeof(tSolicitudESI));
				solicitudESI->mensaje = malloc(100);
				strcpy(solicitudESI->mensaje, "CONEXION OK");
				tPaquete pkgHandshakeESI;
				pkgHandshakeESI.type = C_HANDSHAKE;

				pkgHandshakeESI.length = serializar(pkgHandshakeESI.payload, "%c%s",
						pkgHandshakeESI.type, solicitudESI->mensaje);

				puts("Se envia respuesta al ESI");
				tamanioTotal = enviarPaquete(iSocketComunicacion,
						&pkgHandshakeESI, logCoordinador,
						"Se envia solicitud de ejecucion");
				printf("Se envian %d bytes\n", tamanioTotal);

				*tipoMensaje = DESCONEXION;
				break;

			case P_HANDSHAKE:
				printf("Socket comunicacion: %d \n", iSocketComunicacion);

				puts("HANDSHAKE CON PLANIFICADOR");
				tSolicitudESI* solicitud = malloc(sizeof(tSolicitudESI));
				solicitud->mensaje = malloc(100);
				strcpy(solicitud->mensaje, "OK CONEXION");
				tPaquete pkgHandshake2;
				pkgHandshake2.type = C_HANDSHAKE;

				pkgHandshake2.length = serializar(pkgHandshake2.payload, "%c%s",
						pkgHandshake2.type, solicitud->mensaje);

				puts("Se envia respuesta al Planificador");
				tamanioTotal = enviarPaquete(iSocketComunicacion,
						&pkgHandshake2, logCoordinador,
						"Se envia solicitud de ejecucion");
				printf("Se envian %d bytes\n", tamanioTotal);
				*tipoMensaje = DESCONEXION;
				break;

			case C_HANDSHAKE:
				/* se agrega para que no genere warning
				 * el coordinador no se saluda a sí mismo
				 */
				break;

			case I_HANDSHAKE:
				// TODO se conectó una instancia
				break;
			case DESCONEXION:
				break;

			}
		}
	}
	finalizar(0);
}

//se usa para elegir la instancia con
//la cual operar los comandos SET
//elige la instancia a la cual le va a agregar o modificar la clave
void elegirInstanciaSet() {
	if (!dictionary_has_key(diccionarioClaves, clave)) {
		switch (configuracion->algoritmoDistribucion) {
		case ALGORITMO_LSU:
			elegirInstanciaLSU();
			break;
		case ALGORITMO_EL:
			elegirInstanciaEL();
			break;
		case ALGORITMO_KE:
			elegirInstanciaKE();
			break;
		}
	} else {
		instanciaElegida = dictionary_get(diccionarioClaves, clave);
	}
}

//metodo que se ejecuta cuando la instancia elegida
//responde que guardo correctamente la clave.
//no se debe llamar cuando la clave solamente es modificada
void registrarClaveAgregadaAInstancia() {
	dictionary_put(diccionarioClaves, clave, instanciaElegida);
}

//LEAST SPACE USED
//usa la clave de operacion (variable global)
//setea la instancia elegida (variable global)
void elegirInstanciaLSU() {
}

//EQUITATIVE LOAD
//usa la clave de operacion (variable global)
//setea la instancia elegida (variable global)
void elegirInstanciaEL() {
	if (punteroEL >= instancias->elements_count) {
		punteroEL = 0;
	}
	instanciaElegida = list_get(instancias, punteroEL);
	punteroEL ++;
}

//KEY EXPLICIT
//usa la clave de operacion (variable global)
//setea la instancia elegida (variable global)
void elegirInstanciaKE() {
}

void instanciaDestroyer(t_instancia * instancia) {
	free(instancia->nombre);
	free(instancia);
}

t_instancia * instanciaNew() {
	t_instancia *instancia = malloc(sizeof(t_instancia));
	instancia->nombre = string_new();
	return instancia;
}
