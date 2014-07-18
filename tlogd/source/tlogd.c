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

#include "protocol/tlibc_binary_reader.h"

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
tlog_t					 g_tlog;


static bool on_recviov(tbusapi_t *self, struct iovec *iov, uint32_t iov_num)
{
	bool ret = false;
	uint32_t i;

	for(i = 0;i < iov_num;++i)
	{
    	tlibc_error_code_t    r;

		g_binary_reader.offset = 0;
		g_binary_reader.size = (uint32_t)iov[i].iov_len;
		g_binary_reader.addr = iov[i].iov_base;
		r = tlibc_read_tlog_message(&g_binary_reader.super, &g_message);
    
        if(r != E_TLIBC_NOERROR)
		{
			ERROR_PRINT("tlibc_read_tlog_message(), errno %d", r);
			goto done;
		}
		tlog_write(&g_tlog, &g_message);
    }

	ret = true;
done:
	return ret;
}

static tlibc_error_code_t init()
{
	if(tbusapi_init(&g_tbusapi, g_config.input_tbuskey, TLOGD_IOV_NUM, 0) != E_TLIBC_NOERROR)
	{
		ERROR_PRINT("tbusapi_init failed.");
		goto error_ret;
	}

	if(tlog_init(&g_tlog, &g_config.tlog_config) != E_TLIBC_NOERROR)
	{
		ERROR_PRINT("tlog_init failed.");
		goto error_ret;
	}

    tlibc_binary_reader_init(&g_binary_reader, NULL, 0);
	g_tbusapi.on_recviov = on_recviov;

	return E_TLIBC_NOERROR;
error_ret:
	return E_TLIBC_ERROR;
}

static void fini()
{
	tbusapi_fini(&g_tbusapi);
	tlog_fini(&g_tlog);
}

int main(int argc, char **argv)
{
    int ret = 0;
    
    tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tlogd_config);
	if(init() != E_TLIBC_NOERROR)
	{
		ret = 1;
		goto done;
	}

	if(tapp_loop(TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL, NULL, NULL
				 , tbusapi_process, &g_tbusapi
	             , NULL, NULL) != E_TLIBC_NOERROR)
	{
		ret = 1;
	}

	fini();
done:
    return ret;
}

