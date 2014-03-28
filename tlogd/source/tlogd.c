#include "tlogd.h"
#include "tapp.h"
#include "terrno.h"
#include "tlog_print.h"

#include "tlogd_config_types.h"
#include "tlogd_config_reader.h"
#include <stdio.h>

tlogd_config_t g_config;

static TERROR_CODE process()
{
    return E_TS_WOULD_BLOCK;
}

static TERROR_CODE onsigusr1()
{
    printf("onsigusr1\n");
    return E_TS_NOERROR;
}

static TERROR_CODE onsigusr2()
{
    printf("onsigusr2\n");
    return E_TS_NOERROR;
}



int main(int argc, char **argv)
{
    tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tlogd_config_t);
    return tapp_loop(process, TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, onsigusr1, onsigusr2);
}

