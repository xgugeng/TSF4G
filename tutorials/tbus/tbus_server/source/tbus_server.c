#include "tbusapi.h"
#include "tapp.h"

#include <stdlib.h>
#include <stdio.h>

#define SHM_KEY 123456

static void on_recv(tbusapi_t *self, const char *buf, size_t buf_len)
{
	printf("recv %zu bytes, message:%s\n", buf_len, buf);
}

tbusapi_t g_tbusapi;

int main(int argc, char *argv[])
{
    tlibc_error_code_t ret;

	if(tbusapi_init(&g_tbusapi, SHM_KEY, 1, 0) != E_TLIBC_NOERROR)
	{
		fprintf(stderr, "tbusapi_init failed.\n");
		exit(1);
	}

	g_tbusapi.on_recv = on_recv;
    ret = tapp_loop(TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL, NULL, NULL
                     , tbusapi_process, &g_tbusapi
                     , NULL, NULL);

    tbusapi_fini(&g_tbusapi);
    
    if(ret == E_TLIBC_NOERROR)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

