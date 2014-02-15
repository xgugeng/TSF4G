#ifndef _H_GLOBALS_H
#define _H_GLOBALS_H

#include "tconnd/tconnd_config_types.h"
#include "tdtp_instance.h"
#include "tlog/tlog.h"

#define TCONND_VERSION "0.0.1"

extern tconnd_config_t g_config;

extern tdtp_instance_t g_tdtp_instance;

extern size_t              g_head_size;

extern int g_tdtp_instance_switch;

extern const char* g_config_file;

extern tlog_t g_tlog_instance;



#endif//_H_GLOBALS_H
