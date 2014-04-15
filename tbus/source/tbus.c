#include "tbus.h"
#include "terrno.h"


#include <string.h>


TERROR_CODE tbus_init(tbus_t *tb, size_t size, size_t number)
{
	TERROR_CODE ret = E_TS_NOERROR;

	tb->head_offset = 0;
	tb->tail_offset = 0;
	if(size <= TLIBC_OFFSET_OF(tbus_t, buff))
	{
		ret = E_TS_NO_MEMORY;
		goto done;
	}
	tb->packet_size = (tbus_atomic_size_t)(size + sizeof(tbus_header_t));
	tb->size = (tbus_atomic_size_t)(tb->packet_size * number) + 1;
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
        if(write_size < (tbus_atomic_size_t)sizeof(tbus_header_t))
        {
            if(head_offset != 0)
            {
                tb->tail_offset = 0;
                return tbus_send_begin(tb, buf);
            }
            return 0;            
        }
        else if(write_size < tb->packet_size)
        {
        	tbus_header_t *header = (tbus_header_t*)(tb->buff + tail_offset);
        	header->cmd = e_tbus_cmd_ignore;
            if(head_offset != 0)
            {
                tb->tail_offset = 0;
                return tbus_send_begin(tb, buf);
            }
            return 0;
        }
        else
        {
            *buf = tb->buff + tail_offset + sizeof(tbus_header_t);
        	return write_size - (tbus_atomic_size_t)sizeof(tbus_header_t);
        }
	}
	else		
	{
		write_size = head_offset - tail_offset - 1;
		
        if(write_size < tb->packet_size)
        {
            return 0;
        }
        else
        {
            *buf = tb->buff + tail_offset + sizeof(tbus_header_t);
            return write_size - (tbus_atomic_size_t)sizeof(tbus_header_t);
        }
	}
}


void tbus_send_end(tbus_t *tb, tbus_atomic_size_t len)
{
	tbus_atomic_size_t tail_offset = tb->tail_offset;
	tbus_header_t *header = (tbus_header_t*)(tb->buff + tail_offset);

    //0是空缓存的意思， 所以不能发送0字节
    if(len == 0)
    {
        return;
    }
	header->cmd = e_tbus_cmd_package;
	header->size = len;

	tail_offset += (tbus_atomic_size_t)sizeof(tbus_header_t) + len;

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
        if(read_size < (tbus_atomic_size_t)sizeof(tbus_header_t))
    	{
    	    return 0;
    	}
    	else
    	{
            tbus_header_t *header = (tbus_header_t*)(tb->buff + head_offset);            
    		*buf = tb->buff + sizeof(tbus_header_t) + head_offset;
    		
    		if((size_t)read_size - sizeof(tbus_header_t) < (size_t)header->size)
    		{
    		    return read_size - (tbus_atomic_size_t)sizeof(tbus_header_t);
    		}
    		else
    		{
                return header->size;
            }
    	}
    	
	}
	else
	{
		read_size = tb->size - head_offset - 1;
		if(read_size < sizeof(tbus_header_t))
    	{
            tb->head_offset = 0;
            return tbus_read_begin(tb, buf);
    	}
    	else
    	{
    	    tbus_header_t *header = (tbus_header_t*)(tb->buff + head_offset);
            if(header->cmd == e_tbus_cmd_package)
            {
        		*buf = tb->buff + sizeof(tbus_header_t) + head_offset;
        		
        		if((size_t)read_size - sizeof(tbus_header_t) < (size_t)header->size)
        		{
        		    return read_size - (tbus_atomic_size_t)sizeof(tbus_header_t);
        		}
        		else
        		{
                    return header->size;
                }
            }    
            else
            {
                tb->head_offset = 0;
        	    return tbus_read_begin(tb, buf);
            }
    	}
	}
}


void tbus_read_end(tbus_t *tb, tbus_atomic_size_t len)
{
	tbus_atomic_size_t head_offset = tb->head_offset + (tbus_atomic_size_t)sizeof(tbus_header_t) + len;
	if(head_offset >= tb->size)
	{
		head_offset = 0;
	}
	tb->head_offset = head_offset;
}

/*
static size_t peek(const char *start, const char *limit, tiovec_t *iov, size_t iov_num)
{
	return 0;
}

size_t tbus_peek(tbus_t *tb, tiovec_t *iov, size_t iov_num)
{
	size_t num = 0;

	tbus_atomic_size_t tail = tb->tail_offset;
	tbus_atomic_size_t head = tb->head_offset;
	if(head_offset < tail_offset)
	{
		num += peek(tb->buff + head, tb->buff + tail, iov, iov_num);
	}
	return num;
}

void tbus_peek_over(tbus_t *tb)
{
	return;
}

*/
