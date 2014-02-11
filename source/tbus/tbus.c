#include "tbus/tbus.h"
#include "tcommon/terrno.h"


#include <string.h>

#define CMD_PACKAGE 0x100
#define CMD_IGNORE 0x200
typedef int tbus_header_t;
#define GET_COMMAND(cmd) (cmd & 0xff00)
#define GET_PACKAGE_SIZE(cmd) (cmd & 0xff)
#define BUILD_HEADER(cmd, size) (cmd | size)


TERROR_CODE tbus_init(tbus_t *tb, size_t size)
{
	TERROR_CODE ret = E_TS_NOERROR;

	tb->head_offset = 0;
	tb->tail_offset = 0;
	if(size <= TLIBC_OFFSET_OF(tbus_t, buff))
	{
		ret = E_TS_NO_MEMORY;
		goto done;
	}
	tb->size = size - TLIBC_OFFSET_OF(tbus_t, buff);
	return E_TS_NOERROR;
done:
	return ret;
}


TERROR_CODE tbus_send_begin(tbus_t *tb, char** buf, size_t *len)
{
    TERROR_CODE ret = E_TS_NOERROR;
	size_t write_size;	
	int head_offset = tb->head_offset;
	int tail_offset = tb->tail_offset;

	if(*len + sizeof(tbus_header_t) + 1 > tb->size)
	{
	    ret = E_TS_NO_MEMORY;
		goto done;
	}

	if(head_offset <= tail_offset)
	{
		write_size = tb->size - tail_offset - 1;
	}
	else		
	{
		write_size = head_offset - tail_offset - 1;
	}
	

	if(write_size < sizeof(tbus_header_t) + *len)
	{
		if((head_offset <= tail_offset) && (head_offset != 0))
		{
			tb->tail_offset = 0;
			return tbus_send_begin(tb, buf, len);
		}
		ret = E_TS_WOULD_BLOCK;
		goto done;
	}

	*buf = tb->buff + sizeof(tbus_header_t) + tail_offset;
	*len = write_size - sizeof(tbus_header_t);

done:
    return ret;
}

void tbus_send_end(tbus_t *tb, size_t len)
{
	int head_offset = tb->head_offset;
	int tail_offset = tb->tail_offset;
	tbus_header_t *header = (tbus_header_t*)(tb->buff + head_offset);

	*header = BUILD_HEADER(CMD_PACKAGE, len);

	tail_offset += sizeof(tbus_header_t) + len;
	tb->tail_offset = tail_offset;
}

TERROR_CODE tbus_read_begin(tbus_t *tb, const char** buf, size_t *len)
{
    TERROR_CODE ret = E_TS_NOERROR;
	size_t read_size;
	int tail_offset = tb->tail_offset;
	int head_offset = tb->head_offset;
	tbus_header_t *header = (tbus_header_t*)(tb->buff + head_offset);

	if(head_offset <= tail_offset)
	{
		read_size = tail_offset - head_offset;
	}
	else
	{
		read_size = tb->size - head_offset;
	}

	if(read_size < sizeof(tbus_header_t))
	{
		if(head_offset > tail_offset)
		{
			tb->head_offset = 0;
			return tbus_read_begin(tb, buf, len);
		}
		ret = E_TS_WOULD_BLOCK;
		goto done;
	}


	switch(GET_COMMAND(*header))
	{
	case CMD_IGNORE:
		if(head_offset > tail_offset)
		{
			tb->head_offset = 0;
			return tbus_read_begin(tb, buf, len);
		}
		ret = E_TS_WOULD_BLOCK;
		goto done;
	case CMD_PACKAGE:
		*buf = tb->buff + sizeof(tbus_header_t) + head_offset;
		*len = GET_PACKAGE_SIZE(*header);
		break;
	default:
	    ret = E_TS_ERROR;
		goto done;
	}
	
done:
    return ret;
}

void tbus_read_end(tbus_t *tb, size_t len)
{
	tuint32 head_offset = tb->head_offset + sizeof(tbus_header_t) + len;
	if(head_offset >= tb->size)
	{
		head_offset = 0;
	}
	tb->head_offset = head_offset;
}

