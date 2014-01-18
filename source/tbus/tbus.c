#include "tlibc/platform/tlibc_platform.h"

#include "tserver/tbus/tbus.h"
#include "tserver/terrno.h"

#include <sys/ipc.h>
#include <sys/shm.h>


#include <string.h>

#define CMD_PACKAGE 0x100
#define CMD_IGNORE 0x200
typedef int tbus_header_t;
#define GET_COMMAND(cmd) (cmd & 0xff00)
#define GET_PACKAGE_SIZE(cmd) (cmd & 0xff)
#define BUILD_HEADER(cmd, size) (cmd | size)


TERROR_CODE tbus_init(tbus_t *tb, int size)
{
	TERROR_CODE ret = E_TS_NOERROR;

	tb->head_offset = 0;
	tb->tail_offset = 0;
	if(size < 0)
	{
		ret = E_TS_NO_MEMORY;
		goto ERROR_RET;
	}
	if((unsigned)size <= TLIBC_OFFSET_OF(tbus_t, buff))
	{
		ret = E_TS_NO_MEMORY;
		goto ERROR_RET;
	}
	tb->size = size - TLIBC_OFFSET_OF(tbus_t, buff);
	return E_TS_NOERROR;
ERROR_RET:
	return ret;
}

tbus_t *tbus_open(int shm_key)
{
	int shm_id = shmget(shm_key, 0, 0666);
	return shmat(shm_id, NULL, 0);
}

TERROR_CODE tbus_send(tbus_t *tb, const char* buf, tuint16 len)
{
	size_t write_size;	

	if(sizeof(tbus_header_t) + len + 1 > (unsigned)tb->size)
	{
		goto NO_MEMORY;
	}
	
	if(tb->head_offset <= tb->tail_offset)
	{
		write_size = tb->size - tb->tail_offset - 1;
	}
	else		
	{
		write_size = tb->head_offset - tb->tail_offset - 1;
	}
	

	if(write_size < sizeof(tbus_header_t))
	{
		if((tb->head_offset <= tb->tail_offset) && (tb->head_offset != 0))
		{			
			tb->tail_offset = 0;
			goto AGAIN;		
		}
		goto WOULD_BLOCK;
	}
	else
	{
		tbus_header_t *header = (tbus_header_t*)(tb->buff + tb->tail_offset);
		if(write_size < sizeof(tbus_header_t) + len)
		{
			if((tb->head_offset <= tb->tail_offset) && (tb->head_offset == 0))
			{
				*header = BUILD_HEADER(CMD_IGNORE, 0);
				tb->tail_offset = 0;
				goto AGAIN;
			}
			goto WOULD_BLOCK;
		}
		else
		{
			*header = BUILD_HEADER(CMD_PACKAGE, len);
			memcpy(tb->buff + tb->tail_offset + sizeof(tbus_header_t), buf, len);
			tb->tail_offset += sizeof(tbus_header_t) + len;
		}
	}

	return E_TS_NOERROR;
WOULD_BLOCK:
	return E_TS_WOULD_BLOCK;
AGAIN:
	return E_TS_AGAIN;
NO_MEMORY:
	return E_TS_NO_MEMORY;
}

TERROR_CODE tbus_peek(tbus_t *tb, const char** buf, tuint16 *len)
{
	size_t read_size;

	if(tb->head_offset <= tb->tail_offset)
	{
		read_size = tb->tail_offset - tb->head_offset;
	}
	else
	{
		read_size = tb->size - tb->head_offset;
	}

	if(read_size < sizeof(tbus_header_t))
	{
		if(tb->head_offset > tb->tail_offset)
		{		
			tb->head_offset = 0;
			goto AGAIN;
		}
		goto WOULD_BLOCK;
	}
	else
	{
		tbus_header_t *header = (tbus_header_t*)(tb->buff + tb->head_offset);
		switch(GET_COMMAND(*header))
		{
		case CMD_IGNORE:
			if(tb->head_offset > tb->tail_offset)
			{		
				tb->head_offset = 0;
				goto AGAIN;
			}
			goto WOULD_BLOCK;
		case CMD_PACKAGE:
			*buf = tb->buff + tb->head_offset + sizeof(tbus_header_t);
			*len = GET_PACKAGE_SIZE(*header);
			break;
		default:
			goto ERROR_RET;
		}
	}
	
	return E_TS_NOERROR;
WOULD_BLOCK:
	return E_TS_WOULD_BLOCK;
AGAIN:
	return E_TS_AGAIN;
ERROR_RET:
	return E_TS_ERROR;
}

void tbus_peek_over(tbus_t *tb, tuint16 len)
{
	tb->head_offset += sizeof(tbus_header_t) + len;
	if(tb->head_offset >= tb->size)
	{
		tb->head_offset = 0;
	}
}

