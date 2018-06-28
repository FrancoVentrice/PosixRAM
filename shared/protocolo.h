#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include <stdlib.h>
#include <stdint.h> //para los "int8_t"
#include <string.h>

#define MAX_BUFFER 1024

typedef struct {
	int8_t  type;
	int16_t length;
} __attribute__ ((__packed__)) tHeader;

typedef struct {
	int8_t  type;
	int16_t length;
	char    payload[MAX_BUFFER];
} __attribute__ ((__packed__)) tPaquete;

typedef enum {
	/* Mensajes del ESI */
	E_HANDSHAKE,
	E_SENTENCIA_GET,
	E_SENTENCIA_SET,
	E_SENTENCIA_STORE,
	E_ESI_FINALIZADO,
	E_LINEA_OK,

	/* Mensajes de Planificador */
	P_HANDSHAKE,
	P_RESPUESTA_CONSULTA,
	P_ESTADO_CLAVE,
	P_EJECUTAR_LINEA,
	P_ABORTAR,

	/* Mensajes de Coordinador */
	C_HANDSHAKE,
	C_CONSULTA_OPERACION_GET,
	C_CONSULTA_OPERACION_SET,
	C_CONSULTA_OPERACION_STORE,
	C_RESULTADO_OPERACION,
	C_EJECUTAR_SET,
	C_EJECUTAR_STORE,
	C_EJECUTAR_COMPACTACION,
	C_ESTADO_CLAVE,

	/* Mensajes de Instancia */
	I_HANDSHAKE,
	I_RESULTADO_SET,
	I_RESULTADO_STORE,
	I_COMPACTACION_TERMINADA,
	I_ESTADO_CLAVE,
	I_RESULTADO_ERROR,

	/* Mensajes comunes */
	DESCONEXION,

} tMensaje;

#endif /* PROTOCOLO_H_ */
