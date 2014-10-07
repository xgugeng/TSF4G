#include "tlogd.h"
#include "tapp.h"
#include "tlibc_error_code.h"
#include "tlog_print.h"
#include "tlog.h"
#include "tbusapi.h"

#include "tlogd_config_types.h"
#include "tlogd_config_reader.h"

#include "tlog_message_types.h"
#include "tlog_message_reader.h"

#include "tlibc_binary_reader.h"

#include <string.h>
#include <stdio.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

//不可以超过TBUSAPI_IOV_NUM
#define TLOGD_IOV_NUM 10000

#define MAX_BIND_NUM 65536
#define MYINIT_INTERVAL_S 5

tlogd_config_t           g_config;
tlog_message_t           g_message;
tlibc_binary_reader_t    g_binary_reader;
tbusapi_t				 g_tbusapi;
tbus_t					 *g_itb = NULL;
tlog_t					 g_tlog;


static bool on_recv(tbusapi_t *self, const char *buf, size_t buf_len)
{
	tlibc_error_code_t r;

	g_binary_reader.offset = 0;
	g_binary_reader.size = (uint32_t)buf_len;
	g_binary_reader.addr = buf;
	r = tlibc_read_tlog_message(&g_binary_reader.super, &g_message);

	if(r != E_TLIBC_NOERROR)
	{
		ERROR_PRINT("tlibc_read_tlog_message(), errno %d.", r);
		goto done;
	}

	tlog_write(&g_tlog, &g_message);

done:
	return true;
}

static tlibc_error_code_t init()
{
	g_itb = tbus_at(g_config.input_tbuskey);
	if(g_itb == NULL)
	{
		ERROR_PRINT("tbus_at failed.");
		goto error_ret;
	}

	tbusapi_init(&g_tbusapi, g_itb, NULL, NULL);

	if(tlog_init(&g_tlog, &g_config.tlog_config) != E_TLIBC_NOERROR)
	{
		ERROR_PRINT("tlog_init failed.");
		goto error_ret;
	}

    tlibc_binary_reader_init(&g_binary_reader, NULL, 0);
	g_tbusapi.on_recv = on_recv;

	return E_TLIBC_NOERROR;
error_ret:
	return E_TLIBC_ERROR;
}

static void fini()
{
	tbus_dt(g_itb);
	tlog_fini(&g_tlog);
}

int main(int argc, char **argv)
{
    int ret = 0;
    
	if(tapp_sigaction() != E_TLIBC_NOERROR)
	{
		ret = 1;
		ERROR_PRINT("tapp_sigaction(), errno %d.", errno);
		goto done;
	}

    tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tlogd_config);
	if(init() != E_TLIBC_NOERROR)
	{
		ret = 1;
		goto done;
	}


	if(tapp_loop(TAPP_IDLE_USEC, TAPP_IDLE_LIMIT
				 , tbusapi_process, &g_tbusapi
	             , NULL, NULL) != E_TLIBC_NOERROR)
	{
		ret = 1;
	}

	fini();
done:
    return ret;
}

