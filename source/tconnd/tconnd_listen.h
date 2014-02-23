#ifndef _H_TCONND_LISTEN_H
#define _H_TCONND_LISTEN_H

#include "tcommon/terrno.h"

#include "tconnd/tconnd_socket.h"

extern tconnd_socket_t g_listen;


TERROR_CODE tconnd_listen_init();

TERROR_CODE tconnd_listen();

void tconnd_listen_fini();

#endif//_H_TCONND_LISTEN_H
