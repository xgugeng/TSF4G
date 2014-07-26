#include "tbus.h"
#include "tlibc_error_code.h"


#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>


void tbus_init(tbus_t *tb, size_t size, size_t number)
{
	tb->head_offset = 0;
	tb->tail_offset = 0;
	tb->packet_size = (tbus_atomic_size_t)(size + sizeof(tbus_header_t));
	tb->size = (tbus_atomic_size_t)(tb->packet_size * number);
}

tbus_t *tbus_at(key_t key)
{
	tbus_t *ret = NULL;
	int id = shmget(key, 0, 0666);
	if(id == -1)
	{
		return NULL;
	}
	ret = shmat(id, NULL, 0);
	if((ssize_t)ret == -1)
	{
		return NULL;
	}
	return ret;
}

void tbus_dt(tbus_t *tb)
{
	shmdt(tb);
}

tbus_atomic_size_t tbus_send_begin(tbus_t *tb, char** buf)
{
	tbus_atomic_size_t write_size;
	tbus_atomic_size_t head_offset = tb->head_offset;
	tbus_atomic_size_t tail_offset = tb->tail_offset;


	if(head_offset <= tail_offset)
	{
        write_size = tb->size - tail_offset;
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
	if((tail_offset >= tb->size) && (tb->head_offset != 0))
	{
		tail_offset = 0;
	}

	tb->tail_offset = tail_offset;	
}

static char* peek(char *start, char *limit, struct iovec *iov, size_t *iov_num)
{
	char *ret = NULL;
	char *ch = NULL;
	size_t num = 0;
	for(ch = start; ch < limit;)
	{
    	const tbus_header_t *header = (const tbus_header_t*)ch;
		if(limit - ch < sizeof(tbus_header_t))
		{
			ret = limit;
			goto done;
		}
		if(header->cmd == e_tbus_cmd_ignore)
		{
			ret = limit;
			goto done;
		}

		if(header->cmd != e_tbus_cmd_package)
		{
			ret = NULL;
			goto done;
		}
		if(limit - ch < sizeof(tbus_header_t) + header->size)
		{
			ret = NULL;
			goto done;
		}

		if(num >= *iov_num)
		{
			ret = ch;
			goto done;
		}
		iov[num].iov_base = ch + sizeof(tbus_header_t);
		iov[num].iov_len = header->size;
		++num;
		ch += sizeof(tbus_header_t) + header->size;
	}
	ret = ch;
done:
	*iov_num = num;
	return ret;
}

tbus_atomic_size_t tbus_read_begin(tbus_t *tb, struct iovec *iov, size_t *iov_num)
{
	tbus_atomic_size_t ret = 0;
	char* curr = NULL;
	tbus_atomic_size_t tail = tb->tail_offset;
	tbus_atomic_size_t head = tb->head_offset;
	if(head <= tail)
	{
		curr = peek(tb->buff + head, tb->buff + tail, iov, iov_num);
		if(curr == NULL)
		{
			ret = tail;
			*iov_num = 0;
			goto done;
		}
	}
	else
	{
		size_t seg0_num = 0;
		size_t seg1_num = 0;
		char* limit = tb->buff + tb->size;

		seg0_num = *iov_num;
		curr = peek(tb->buff + head, limit, iov, &seg0_num);
		if(curr == NULL)
		{
			ret = tail;
			*iov_num = 0;
			goto done;
		}

		if(curr == limit)
		{
			seg1_num = *iov_num - seg0_num;
			curr = peek(tb->buff, tb->buff + tail, iov + seg0_num, &seg1_num);
			if(curr == NULL)
			{
				ret = tail;
				*iov_num = 0;
				goto done;
			}
		}
		*iov_num = seg0_num + seg1_num;
	}
	
	ret = (tbus_atomic_size_t)(curr - tb->buff);
done:
	return ret;
}

void tbus_read_end(tbus_t *tb, tbus_atomic_size_t head)
{
	tb->head_offset = head;
}

