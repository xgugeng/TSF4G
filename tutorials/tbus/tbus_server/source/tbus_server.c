#include "tbusapi.h"
#include "tapp.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define SHM_KEY 123456

static bool on_recv(tbusapi_t *self, const char *buf, size_t buf_len)
{
	printf("recv %zu bytes, message:%s\n", buf_len, buf);
	return true;
}

tbusapi_t g_tbusapi;
tbus_t *g_itb;

int main(int argc, char *argv[])
{
    tlibc_error_code_t ret;

	if(tapp_sigaction() != E_TLIBC_NOERROR)
	{
		fprintf(stderr, "tapp_sigaction failed.\n");
		exit(1);
	}

	g_itb = tbus_at(SHM_KEY);
	if(g_itb == NULL)
	{
		fprintf(stderr, "tbusapi_init failed.\n");
		exit(1);
	}

	tbusapi_init(&g_tbusapi, g_itb, 0, NULL);

	g_tbusapi.on_recv = on_recv;
    ret = tapp_loop(TAPP_IDLE_USEC, TAPP_IDLE_LIMIT
                     , tbusapi_process, &g_tbusapi
                     , NULL, NULL);

	tbus_dt(g_itb);
    
    if(ret == E_TLIBC_NOERROR)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

