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


tbus_atomic_size_t tbus_send_begin(tbus_t *tb, char** buf)
{
	tbus_atomic_size_t write_size;
	tbus_atomic_size_t head_offset = tb->head_offset;
	tbus_atomic_size_t tail_offset = tb->tail_offset;

	if(head_offset <= tail_offset)
	{
        write_size = tb->size - tail_offset - 1;
        if(write_size < (tbus_atomic_size_t)sizeof(tbus_header_s))
        {
            if(head_offset != 0)
            {
                tb->tail_offset = 0;
                tail_offset = 0;
                write_size = head_offset - 1;
            }
        }
        else if(head_offset > (size_t)write_size + 1)
        {
            tbus_header_s *header = (tbus_header_s*)(tb->buff + tail_offset);
            header->cmd = e_tbus_cmd_ignore;
            
            tb->tail_offset = 0;
            tail_offset = 0;            
            write_size = head_offset - 1;
        }       
	}
	else		
	{
		write_size = head_offset - tail_offset - 1;        
	}


	if(write_size < (tbus_atomic_size_t)sizeof(tbus_header_s))
	{
	    return 0;
	}
	else
	{
    	*buf = tb->buff + tail_offset + sizeof(tbus_header_s);
    	return write_size - (tbus_atomic_size_t)sizeof(tbus_header_s);
	}
}

void tbus_send_end(tbus_t *tb, tbus_atomic_size_t len)
{
	tbus_atomic_size_t tail_offset = tb->tail_offset;
	tbus_header_s *header = (tbus_header_s*)(tb->buff + tail_offset);

    if(len == 0)
    {
        return;
    }
	header->cmd = e_tbus_cmd_package;
	header->size = len;

	tail_offset += (tbus_atomic_size_t)sizeof(tbus_header_s) + len;

	tb->tail_offset = tail_offset;	
}

tbus_atomic_size_t tbus_read_begin(tbus_t *tb, char** buf)
{
	tbus_atomic_size_t read_size;
	tbus_atomic_size_t tail_offset = tb->tail_offset;
	tbus_atomic_size_t head_offset = tb->head_offset;

	if(head_offset <= tail_offset)
	{
		read_size = tail_offset - head_offset;
	}
	else
	{
		read_size = tb->size - head_offset - 1;
		if(read_size < sizeof(tbus_header_s))
    	{
            tb->head_offset = 0;
            head_offset = 0;
            read_size = tail_offset;       
    	}
    	else
    	{
    	    tbus_header_s *header = (tbus_header_s*)(tb->buff + head_offset);
            if(header->cmd == e_tbus_cmd_ignore)
            {
        	    tb->head_offset = 0;
        	    head_offset = 0;
        	    read_size = tail_offset - head_offset;
            }
    	}
	}

	if(read_size < (tbus_atomic_size_t)sizeof(tbus_header_s))
	{
	    return 0;
	}
	else
	{
        tbus_header_s *header = (tbus_header_s*)(tb->buff + head_offset);
        
		*buf = tb->buff + sizeof(tbus_header_s) + head_offset;
		if(read_size < (size_t)header->size + sizeof(tbus_header_s))
		{
		    return read_size - sizeof(tbus_header_s);
		}
		else
		{
            return header->size;
        }
	}
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

