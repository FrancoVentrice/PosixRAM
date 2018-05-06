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
	int maxSock;
	int iSocketEscucha;
	int iSocketComunicacion;
	int tamanioTotal = 0;


	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log
	iSocketEscucha = crearSocketEscucha(puertoEscucha, logger);

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
				logger);

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
						&pkgHandshakeESI, logger,
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
						&pkgHandshake2, logger,
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
				//usar nuevaInstanciaConectada(id, socket);
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
//
//
//To Do: la respuesta de una instancia a un SET deberia devolver la cantidad
//de entradas disponibles asi el valor se mantiene actualizado para este algoritmo
//
//
void elegirInstanciaLSU() {
	int i;
	instanciaElegida = list_get(instancias, 0);
	for (i = 1; i < instancias->elements_count; i++) {
		t_instancia *iinstancia = list_get(instancias, i);
		if (iinstancia->cantidadDeEntradasDisponibles > instanciaElegida->cantidadDeEntradasDisponibles) {
			instanciaElegida = iinstancia;
		}
	}
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
//valor de 'a' es 97
//valor de 'z' es 122
//122 - 97 = 25 letras a ser distribuidas entre las instancias
void elegirInstanciaKE() {
	if (instancias->elements_count == 0) {
		return;
	}
	int letrasPorInstancia = 25 / instancias->elements_count;
	if (25 % instancias->elements_count > 0) {
		letrasPorInstancia++;
	}
	//'a' tendria valor 1, 'b' 2, y asi
	int valorPrimerCaracter = clave[0] - 96;
	int indexInstancia = valorPrimerCaracter / letrasPorInstancia;
	if (valorPrimerCaracter % letrasPorInstancia == 0) {
		indexInstancia--;
	}
	instanciaElegida = list_get(instancias, indexInstancia);
}

void instanciaDestroyer(t_instancia * instancia) {
	free(instancia->nombre);
	free(instancia);
}

t_instancia * instanciaNew() {
	t_instancia *instancia = malloc(sizeof(t_instancia));
	instancia->nombre = string_new();
	instancia->cantidadDeEntradasDisponibles = configuracion->cantidadDeEntradas;
	return instancia;
}

//cuando entra una nueva instancia, se fija si ya existe y la
//tiene que actualizar, o si crea una nueva y la agrega a la lista
void nuevaInstanciaConectada(int id, int socket) {
	int i;
	for (i = 0; i < instancias->elements_count; i++) {
		t_instancia *instancia = list_get(instancias, i);
		if (instancia->id == id) {
			instancia->socket = socket;
			return;
		}
	}
	t_instancia *instancia = instanciaNew();
	instancia->id = id;
	instancia->socket = socket;
	list_add(instancias, instancia);
}
