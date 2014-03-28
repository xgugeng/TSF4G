#include "tlogd.h"
#include "tapp.h"
#include "terrno.h"
#include "tlog_print.h"

#include "tlogd_config_types.h"
#include "tlogd_config_reader.h"


tlogd_config_t g_config;

static TERROR_CODE process()
{
    return E_TS_WOULD_BLOCK;
}

int main(int argc, char **argv)
{
    tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tlogd_config_t);
    return tapp_loop(process, 1000, 30);
}

