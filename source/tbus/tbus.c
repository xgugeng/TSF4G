#include "tlibc/platform/tlibc_platform.h"

#include "tserver/tbus/tbus.h"
#include "tserver/terrno.h"


#define get_addr(tb, offset) (tb->buff + offset)

//返回通道内有多少数据
#define get_size(tb) \
	(tb->head_offset < tb->tail_offset? tb->tail_offset - tb->head_offset\
		: tb->size - tb->head_offset + tb->tail_offset)

//返回通道还可以容纳多少数据
#define get_room(tb) (tb->size - get_size(tb) - 1)




//返回从任意位置开始可读的大小
int get_read_size(tbus_t* tb, int offset)
{
	int ret;
	int head_offset = tb->head_offset;
	int tail_offset = tb->tail_offset;

	if(head_offset <= tail_offset)
	{
		if((offset >= head_offset) && (offset < tail_offset))
		{
			ret = tail_offset - offset;
		}
		else
		{
			ret = 0;
		}
	}
	else
	{
		if(offset >= head_offset)
		{
			ret = tb->size - offset;
		}
		else if(offset < tail_offset)
		{
			ret = tail_offset - offset;
		}
		else
		{
			ret  = 0;
		}
	}
	return ret;
}

//返回从任意位置开始可写的大小
int get_write_size(tbus_t* tb, int offset)
{
	int ret;
	int head_offset = tb->head_offset;
	int tail_offset = tb->tail_offset;

	if(head_offset <= tail_offset)
	{
		if(offset < head_offset)
		{
			ret = head_offset - offset - 1;
		}
		else if(offset >= tail_offset)
		{
			ret = tb->size - offset;
			if(head_offset == 0)
			{
				--ret;
			}
		}
		else
		{
			ret = 0;
		}
	}
	else
	{
		if((offset >= tail_offset) && (offset < head_offset))
		{
			ret = head_offset - offset - 1;
		}
		else
		{
			ret = 0;
		}
	}
	return ret;
}






//把指针向后移动
int tbus_forward_pos(tbus_t* tb, int offset, int distance)
{
	offset += distance;
	while(offset >= tb->size)
	{
		offset -= tb->size;
	}
	return offset;
}

//把指针向前移动
int tbus_backward_pos(tbus_t* tb, int offset, int distance)
{
	offset -= distance;
	while(offset < 0)
	{
		offset += tb->size;
	}
	return offset;
}

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


