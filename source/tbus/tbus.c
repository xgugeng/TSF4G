#include "tbus/tbus.h"
#include "tcommon/terrno.h"


#include <string.h>


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
	tbus_header_s *header = (tbus_header_s*)(tb->buff + tail_offset);

	if(*len + sizeof(tbus_header_s) + 1 > tb->size)
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
	

	if(write_size < sizeof(tbus_header_s) + *len)
	{
		if((head_offset <= tail_offset) && (head_offset != 0))
		{
		    if(write_size >= sizeof(tbus_header_s))
		    {
                header->cmd = e_tbus_cmd_ignore;
                header->size = 0;
		    }
			tb->tail_offset = 0;
			return tbus_send_begin(tb, buf, len);
		}
		ret = E_TS_WOULD_BLOCK;
		goto done;
	}

	*buf = tb->buff + tail_offset + sizeof(tbus_header_s);
	*len = write_size - sizeof(tbus_header_s);

done:
    return ret;
}

void tbus_send_end(tbus_t *tb, size_t len)
{
	int tail_offset = tb->tail_offset;
	tbus_header_s *header = (tbus_header_s*)(tb->buff + tail_offset);

	header->cmd = e_tbus_cmd_package;
	header->size = len;

	tail_offset += sizeof(tbus_header_s) + len;
	tb->tail_offset = tail_offset;
}

TERROR_CODE tbus_read_begin(tbus_t *tb, const char** buf, size_t *len)
{
    TERROR_CODE ret = E_TS_NOERROR;
	size_t read_size;
	int tail_offset = tb->tail_offset;
	int head_offset = tb->head_offset;
	tbus_header_s *header = (tbus_header_s*)(tb->buff + head_offset);

	if(head_offset <= tail_offset)
	{
		read_size = tail_offset - head_offset;
	}
	else
	{
		read_size = tb->size - head_offset;
	}

	if(read_size < sizeof(tbus_header_s))
	{
		if(head_offset > tail_offset)
		{
			tb->head_offset = 0;
			return tbus_read_begin(tb, buf, len);
		}
		ret = E_TS_WOULD_BLOCK;
		goto done;
	}


	switch(header->cmd)
	{
	case e_tbus_cmd_ignore:
		if(head_offset > tail_offset)
		{
			tb->head_offset = 0;
			return tbus_read_begin(tb, buf, len);
		}
		ret = E_TS_WOULD_BLOCK;
		goto done;
	case e_tbus_cmd_package:
		*buf = tb->buff + sizeof(tbus_header_s) + head_offset;
		*len = header->size;
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
	tuint32 head_offset = tb->head_offset + sizeof(tbus_header_s) + len;
	if(head_offset >= tb->size)
	{
		head_offset = 0;
	}
	tb->head_offset = head_offset;
}

