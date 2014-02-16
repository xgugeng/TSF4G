#ifndef _H_TCONND_LISTEN_H
#define _H_TCONND_LISTEN_H

#include "tcommon/terrno.h"

extern int g_listenfd;

TERROR_CODE tconnd_listen_init();

TERROR_CODE process_listen();

void tconnd_listen_fini();

#endif//_H_TCONND_LISTEN_H
