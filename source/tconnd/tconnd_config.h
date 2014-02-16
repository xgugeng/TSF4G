#ifndef _H_TCONND_CONFIG_H
#define _H_TCONND_CONFIG_H

#include "tcommon/terrno.h"

#include "tconnd/tconnd_config_types.h"

extern tconnd_config_t g_config;

TERROR_CODE tconnd_config_init(const char* config_file);

#endif//_H_TCONND_CONFIG_H

