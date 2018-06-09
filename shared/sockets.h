#ifndef LIBSOCKETS_H_
#define LIBSOCKETS_H_

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#include <commons/log.h>
#include "protocolo.h"

int crearSocketEscucha(int, t_log*);

int enviarPaquete(int , tPaquete* , t_log* , char* );

int recibirPaquete(int , tMensaje* , char** , t_log* , char* );

signed int getConnection(fd_set *, int *, int , tMensaje *, char** , t_log* );

signed int getConnectionTimeOut(fd_set *, int *, int , tMensaje *, char** , struct timeval *, t_log* );

signed int multiplexar(fd_set *, fd_set *, int *, tMensaje* , char** , t_log* );

signed int multiplexarTimed(fd_set *, fd_set *, int *, tMensaje* , char** , t_log* , int, int);

signed int connectToServer(char *, int , t_log *);

int desconectarseDe(int);

int getNewConnection(int , fd_set *, int *);

#endif /* LIBSOCKETS_H_ */
