#include "tbus/tbus.h"
#include "tcommon/terrno.h"


#include <string.h>


TERROR_CODE tbus_init(tbus_t *tb, tbus_atomic_size_t size)
{
	TERROR_CODE ret = E_TS_NOERROR;

	tb->head_offset = 0;
	tb->tail_offset = 0;
	if(size <= TLIBC_OFFSET_OF(tbus_t, buff))
	{
		ret = E_TS_NO_MEMORY;
		goto done;
	}
	tb->size = (size - (tbus_atomic_size_t)TLIBC_OFFSET_OF(tbus_t, buff));
	return E_TS_NOERROR;
done:
	return ret;
}


TERROR_CODE tbus_send_begin(tbus_t *tb, char** buf, tbus_atomic_size_t *len)
{
    TERROR_CODE ret = E_TS_NOERROR;
	tbus_atomic_size_t write_size;	
	tbus_atomic_size_t head_offset = tb->head_offset;
	tbus_atomic_size_t tail_offset = tb->tail_offset;
	tbus_header_s *header = (tbus_header_s*)(tb->buff + tail_offset);

	if(*len + (tbus_atomic_size_t)sizeof(tbus_header_s) + 1 > tb->size)
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
	

	if(write_size < (tbus_atomic_size_t)sizeof(tbus_header_s) + *len)
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
		ret = E_TS_TBUS_NOT_ENOUGH_SPACE;
		goto done;
	}

	*buf = tb->buff + tail_offset + sizeof(tbus_header_s);
	*len = write_size - (tbus_atomic_size_t)sizeof(tbus_header_s);

done:
    return ret;
}

void tbus_send_end(tbus_t *tb, tbus_atomic_size_t len)
{
	tbus_atomic_size_t tail_offset = tb->tail_offset;
	tbus_header_s *header = (tbus_header_s*)(tb->buff + tail_offset);

	header->cmd = e_tbus_cmd_package;
	header->size = len;

	tail_offset += (tbus_atomic_size_t)sizeof(tbus_header_s) + len;
	tb->tail_offset = tail_offset;
}

TERROR_CODE tbus_read_begin(tbus_t *tb, char** buf, tbus_atomic_size_t *len)
{
    TERROR_CODE ret = E_TS_NOERROR;
	tbus_atomic_size_t read_size;
	tbus_atomic_size_t tail_offset = tb->tail_offset;
	tbus_atomic_size_t head_offset = tb->head_offset;
	tbus_header_s *header = (tbus_header_s*)(tb->buff + head_offset);

	if(head_offset <= tail_offset)
	{
		read_size = tail_offset - head_offset;
	}
	else
	{
		read_size = tb->size - head_offset - 1;
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

void tbus_read_end(tbus_t *tb, tbus_atomic_size_t len)
{
	tbus_atomic_size_t head_offset = tb->head_offset + (tbus_atomic_size_t)sizeof(tbus_header_s) + len;
	if(head_offset >= tb->size)
	{
		head_offset = 0;
	}
	tb->head_offset = head_offset;
}

