#ifndef _IPCPROTOCOL_H
#define _IPCPROTOCOL_H

/* UTN FRBA
 * Sistemas Operativos
 * TP-1C-2018-ReDistinto
 * (c) PosixRAM */

/* Inter Process Communication (IPC)

 Se utilizará el siguiente protocolo para todas las comunicaciones.

 REQUEST
 DescriptorID (16 b) identificador único del mensaje
 PayloadDescriptor (1 b) identificador de tipo de mensaje
 PayloadLenght (4 b) longitud del mensaje (sin contar el header)
 Payload (variable) mensaje enviado

 RESPONSE
 DescriptorID (16 b) identificador del mensaje correspondiente al request
 PayloadDescriptor (1 b) identificador de tipo de mensaje
 PayloadLenght (4 b) longitud del mensaje (sin contar el header)
 Payload mensaje enviado, longitud variable */

/* Handshake

 REQUEST
 DescriptorID : identificador único
 PayloadDescriptor : process id
 PayloadLenght : 0
 Payload : sin payload

 RESPONSE
 DescriptorID : identificador único del request
 PayloadDescriptor : 1 ok | 0 no ok
 PayloadLenght : 0
 Payload : sin payload
 */

#define int int32_t

// mensajes
#define MSGFAIL 0x00
#define MSGOK 0x01
#define PING 0x10
#define PONG 0x11

// mensajes ESI -> Planificador

// mensajes Planificador -> ESI

// mensajes ESI -> Coordinador

// mensajes Coordinador -> Esi

// mensajes Planificador -> Coordinador

// mensajes Coordinador -> Planificador

// mensajes Instancia -> Coordinador

// mensajes Coordinador -> Instancia


// header struct (21 bytes)
typedef struct header_ipc
{
    unsigned char ucDescriptorID[16];
    unsigned char ucPayloadDescriptor;
    unsigned int uiPayloadLength;
} HEADERIPC;

void crearHeader(HEADERIPC *cabecera, unsigned char ucPayloadDescriptor, unsigned int uiPayloadLength);
void generarID(unsigned char* ucpID);

#endif
