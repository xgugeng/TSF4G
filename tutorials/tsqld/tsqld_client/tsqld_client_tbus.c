#include "tsqld_client_tbus.h"
#include "tlog/tlog_print.h"
#include "tlog/tlog_log.h"
#include "tsqld_protocol/tsqld_protocol_writer.h"
#include "tlibc/protocol/tlibc_binary_writer.h"
#include "tlibc/protocol/tlibc_binary_reader.h"
#include "tsqld_protocol/tsqld_protocol_reader.h"
#include "tbus/tbus.h"
#include "tcommon/terrno.h"
#include <string.h>
#include <assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "tsqld_client.h"
#include <errno.h>
#include "tlibc/core/tlibc_list.h"

tbus_t *g_itb;
tbus_t *g_otb;

static char *g_write_start = NULL;           //开始写入的指针
static char *g_write_limit = NULL;           //结束写入的指针
static tbus_atomic_size_t g_write_size = 0;  //总共可以写入的长度
static char *g_write_cur = NULL;             //当前写入的指针

#define WRITE_LIMIT 1 * 1000000    //最大缓存长度
#define INPUT_TBUSKEY 20002
#define OUTPUT_TBUSKEY 20001

TERROR_CODE tsqld_client_tbus_init()
{
    TERROR_CODE ret;
    int input_tbusid;
    int output_tbusid;

	input_tbusid = shmget(INPUT_TBUSKEY, 0, 0666);
	if(input_tbusid == -1)
	{
	    ERROR_LOG("shmget errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}
	g_itb = shmat(input_tbusid, NULL, 0);
	if(g_itb == NULL)
	{
        ERROR_LOG("shmat errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}

	output_tbusid = shmget(OUTPUT_TBUSKEY, 0, 0666);
	if(output_tbusid == -1)
	{
        ERROR_LOG("shmget errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}
	g_otb = shmat(output_tbusid, NULL, 0);
	if(g_otb == NULL)
	{
        ERROR_LOG("shmat errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}

    g_write_start = NULL;
    g_write_limit = NULL;
    g_write_size = 0;
    g_write_cur = NULL;

    return E_TS_NOERROR;
shmdt_input:
    shmdt(g_itb);
done:
    return ret;
}

void tsqld_client_tbus_fini()
{
    shmdt(g_itb);
    shmdt(g_otb);
}

void tsqld_client_tbus_send(const tsqld_protocol_t *head, const char* data, size_t data_size)
{
    tbus_atomic_size_t total_size= 0;
    size_t head_size;
    char head_buff[sizeof(tsqld_protocol_t)];    
    TLIBC_BINARY_WRITER bw;
    TLIBC_ERROR_CODE r;
    
    tlibc_binary_writer_init(&bw, head_buff, sizeof(head_buff));
    r = tlibc_write_tsqld_protocol_t(&bw.super, head);
    if(r != E_TLIBC_NOERROR)
    {
        ERROR_LOG("tlibc_write_tsqld_protocol_t reutrn [%d].", r);
        goto done;
    }
    

    head_size = bw.offset;
    total_size = (tbus_atomic_size_t)(head_size + data_size);
      

    if(g_write_size == 0)
    {
        g_write_size = total_size;
        if(tbus_send_begin(g_otb, &g_write_start, &g_write_size) != E_TS_NOERROR)
        {
            g_write_size = 0;
            ERROR_LOG("tbus no space, drop mid [%d] data_size [%u].", head->message_id, data_size);
            return;
        }
        g_write_limit = g_write_start + g_write_size;
        g_write_cur = g_write_start;
    }
    else
    {
        if(g_write_limit - g_write_cur < total_size)
        {
            tbus_send_end(g_otb, (tbus_atomic_size_t)(g_write_cur - g_write_start));
            
            g_write_size = total_size;
            if(tbus_send_begin(g_otb, &g_write_start, &g_write_size) != E_TS_NOERROR)
            {
                g_write_size = 0;
                ERROR_LOG("tbus no space, drop mid [%d] data_size [%u].", head->message_id, data_size);
                return;
            }
            g_write_limit = g_write_start + g_write_size;
            g_write_cur = g_write_start;
        }
    }
    
    assert(g_write_size != 0);
    assert(g_write_cur != NULL);
    assert(g_write_limit - g_write_cur >= total_size);    


    memcpy(g_write_cur, head_buff, head_size);         
    g_write_cur += head_size;
    if(data)
    {
        memcpy(g_write_cur, data, data_size);
        g_write_cur += data_size;
    }
    
    if(g_write_cur - g_write_start >= WRITE_LIMIT)
    {
        tbus_send_end(g_otb, (tbus_atomic_size_t)(g_write_cur - g_write_start));
        g_write_size = 0;
    }

done:
    return;
}

void tsqld_client_tbus_flush()
{
    if(g_write_size != 0)
    {
        tbus_send_end(g_otb, (tbus_atomic_size_t)(g_write_cur - g_write_start));
        g_write_size = 0;
    }
}

static void tsqld_client_tbus_recv_pkg(const tsqld_protocol_t *req)
{
}

static void tsqld_client_tbus_send_pkg()
{
}

static TERROR_CODE tsqld_client_tbus_recv()
{
    TERROR_CODE ret = E_TS_NOERROR;
    char*message, *cur, *message_limit;
    tbus_atomic_size_t message_len = 0;
        
    ret = tbus_read_begin(g_itb, &message, &message_len);
    if(ret == E_TS_WOULD_BLOCK)
    {
        goto done;
    }
    else if(ret == E_TS_BAD_PACKAGE)
    {
        ERROR_LOG("tbus receive a bad package.");
        goto read_end;
    }
    else if(ret != E_TS_NOERROR)
    {
        ERROR_LOG("tbus_read_begin return %d", ret);
        goto done;
    }

    message_limit = message + message_len;

    for(cur = message; cur < message_limit;)
    {
        TLIBC_BINARY_READER br;
        TLIBC_ERROR_CODE r;
        tsqld_protocol_t head;
        
        tlibc_binary_reader_init(&br, cur, (uint32_t)(message_limit - cur));
        r = tlibc_read_tsqld_protocol_t(&br.super, &head);
        if(r != E_TLIBC_NOERROR)
        {
            ERROR_LOG("tlibc_write_tsqld_protocol_t reutrn [%d].", r);
            goto read_end;
        }
        tsqld_client_tbus_recv_pkg(&head);
        cur += br.offset;
    }

    assert(cur == message_limit);
read_end:
    tbus_read_end(g_itb, message_len);
done:
    return ret;
}

TERROR_CODE tsqld_client_tbus_proc()
{
	tsqld_client_tbus_send_pkg();

	while(tsqld_client_tbus_recv() == E_TS_WOULD_BLOCK)
	{
		usleep(1000);
	}

	return E_TS_NOERROR;
}

