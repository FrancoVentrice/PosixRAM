#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_


#include <stdlib.h>
#include <stdint.h> //para los "int8_t"
#include <string.h>

#define TAM_BLOQUE 1048576
#define MAX_BUFFER 1024
#define MAX_BUFFER_GRANDE (TAM_BLOQUE + MAX_BUFFER)
#define BLOQUE_SIZE TAM_BLOQUE
#define CANT_COPIAS 2

typedef struct {
	int8_t  type;
	int16_t length;
} __attribute__ ((__packed__)) tHeader;

typedef struct {
	int8_t  type;
	int16_t length;
	char    payload[MAX_BUFFER];
} __attribute__ ((__packed__)) tPaquete;

typedef struct {
	int8_t  type;
	int32_t length;
} __attribute__ ((__packed__)) tHeaderGrande;

typedef struct {
	int8_t  type;
	int32_t length;
	char    payload[MAX_BUFFER_GRANDE];
} __attribute__ ((__packed__)) tPaqueteGrande;

/*
 * Formato del tipo del paquete:
 * 		[emisor]_[mensaje]
 * Emisor:
 * 		Y : YAMA
 * 		M : MASTER
 * 		W : WORKER
 * 		D : DATANODE
 * 		F : FILESYSTEM
 *
 */
typedef enum {
	/* Mensajes del ESI */
	E_HANDSHAKE,



	/* Mensajes de Planificador */
	P_HANDSHAKE,



	/* Mensajes de Coordindor */
	C_HANDSHAKE,

	/* Mensajes de Instancia */
	I_HANDSHAKE,





	/* Mensajes comunes */
	DESCONEXION,

} tMensaje;








#endif /* PROTOCOLO_H_ */
