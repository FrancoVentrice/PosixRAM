/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

#include "ESI.h"
#include "..//shared/protocolo.h"
#include "..//shared/sockets.c"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

int main(int argn, char *argv[]) {
	cargarConfiguracion();
	inicarConexiones();

}

void finalizar(int codigo) {
	limpiarConfiguracion();
	exit(codigo);
}
void inicarConexiones() {
		int bytesEnviados;
		int socketServidor = connectToServer(configuracion->ipPlanificador,
				configuracion->puertoPlanificador, logger);

		tSolicitudESI* solicitud = malloc(sizeof(tSolicitudESI));
		solicitud->mensaje = malloc(100);
		strcpy(solicitud->mensaje, "HOLA SOY ESI!!!");
		tPaquete pkgHandshake;
		pkgHandshake.type = E_HANDSHAKE;
		tRespuestaPlanificador *respuesta = malloc(
				sizeof(tRespuestaPlanificador));

		pkgHandshake.length = serializar(pkgHandshake.payload, "%c%s",
				pkgHandshake.type, solicitud->mensaje);

		puts("Se envia solicitud al Planificador");
		bytesEnviados = enviarPaquete(socketServidor, &pkgHandshake, logger,
				"Se envia solicitud de ejecucion");
		printf("Se envian %d bytes\n", bytesEnviados);

		//Manejo los mensajes del Planificador o del Coordinador
		char * sPayloadRespuestaHand = malloc(100);

		tMensaje tipoMensaje;

		int bytesRecibidos = recibirPaquete(socketServidor, &tipoMensaje,
				&sPayloadRespuestaHand, logger, "Hand Respuesta");
		printf("RECIBIDOS:%d\n", bytesRecibidos);

		switch (tipoMensaje) {

		case P_HANDSHAKE:

			respuesta->mensaje = malloc(10);
			char encabezado_mensaje;

			deserializar(sPayloadRespuestaHand, "%c%s", &encabezado_mensaje,
					respuesta->mensaje);
			printf("RESPUESTA: %s \n", respuesta->mensaje);

			//Conexion al Coordinador
			int puertoConexion2 = 8001;
			int bytesEnviados;
			logger = log_create("cliente.log", "CLIENTE", 1, LOG_LEVEL_TRACE);
			int socketServidor2 = connectToServer("127.0.0.1", puertoConexion2,
					logger);

			tSolicitudESI* solicitud2 = malloc(sizeof(tSolicitudESI));
			solicitud2->mensaje = malloc(100);
			strcpy(solicitud2->mensaje, "HOLA SOY ESI!!!");
			tPaquete pkgHandshake2;
			pkgHandshake2.type = E_HANDSHAKE;
			int tamanioTotal = 0;

			pkgHandshake2.length = serializar(pkgHandshake2.payload, "%c%s",
					pkgHandshake2.type, solicitud2->mensaje);

			puts("Se envia solicitud de ejecucion al Coordinador");
			tamanioTotal = enviarPaquete(socketServidor2, &pkgHandshake2,
					logger, "Se envia solicitud de ejecucion");
			printf("Se envian %d bytes\n", tamanioTotal);

			break;

		}

		finalizar(0);
	}



