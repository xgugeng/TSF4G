#include "protocol/tlibc_compact_reader.h"
#include "platform/tlibc_platform.h"
#include "core/tlibc_util.h"
#include "protocol/tlibc_abstract_reader.h"
#include "core/tlibc_error_code.h"

#include <string.h>
#include <assert.h>

static tlibc_error_code_t tlibc_compact_varint16_decode(const char *buff_ptr, uint32_t *buff_size, uint16_t *result)
{
	uint8_t b;

	if(*buff_size < 1)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 0);
	*result = (b & 0x7F);
	if (!(b & 0x80))
	{
		*buff_size = 1;
		goto done;
	}

	if(*buff_size < 2)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 1);
	*result |= (b & 0x7F) << 7;
	if (!(b & 0x80))
	{
		*buff_size = 2;
		goto done;
	}

	if(*buff_size < 3)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 2);
	*result |= (b & 0x7F) << 14;
	if (!(b & 0x80))
	{
		*buff_size = 3;
		goto done;
	}

	return E_TLIBC_ERROR;
done:
	return E_TLIBC_NOERROR;

not_enough_byte_size:
	return E_TLIBC_OUT_OF_MEMORY;
}


static tlibc_error_code_t tlibc_compact_varint32_decode(const char *buff_ptr, uint32_t *buff_size, uint32_t *result)
{
	uint8_t b;

	if(*buff_size < 1)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 0);
	*result = (b & 0x7F);
	if (!(b & 0x80))
	{
		*buff_size = 1;
		goto done;
	}

	if(*buff_size < 2)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 1);
	*result |= (b & 0x7F) << 7;
	if (!(b & 0x80))
	{
		*buff_size = 2;
		goto done;
	}

	if(*buff_size < 3)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 2);
	*result |= (b & 0x7F) << 14;
	if (!(b & 0x80))
	{
		*buff_size = 3;
		goto done;
	}

	if(*buff_size < 4)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 3);
	*result |= (b & 0x7F) << 21;
	if (!(b & 0x80))
	{
		*buff_size = 4;
		goto done;
	}

	if(*buff_size < 5)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 4);
	*result |= (b & 0x7F) << 28;
	if (!(b & 0x80))
	{
		*buff_size = 5;
		goto done;
	}	

	return E_TLIBC_ERROR;
done:
	return E_TLIBC_NOERROR;

not_enough_byte_size:
	return E_TLIBC_OUT_OF_MEMORY;
}


static tlibc_error_code_t tlibc_compact_varint64_decode(const char *buff_ptr, uint32_t *buff_size, uint64_t *result)
{
	uint8_t b;

	uint32_t par0 = 0;
	uint32_t par1 = 0;
	uint32_t par2 = 0;

	//par0
	if(*buff_size < 1)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 0);
	par0 = (b & 0x7F);
	if (!(b & 0x80))
	{
		*buff_size = 1;
		goto done;
	}

	if(*buff_size < 2)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 1);
	par0 |= (b & 0x7F) << 7;
	if (!(b & 0x80))
	{
		*buff_size = 2;
		goto done;
	}

	if(*buff_size < 3)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 2);
	par0 |= (b & 0x7F) << 14;
	if (!(b & 0x80))
	{
		*buff_size = 3;
		goto done;
	}

	if(*buff_size < 4)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 3);
	par0 |= (b & 0x7F) << 21;
	if (!(b & 0x80))
	{
		*buff_size = 4;
		goto done;
	}

	//par1
	if(*buff_size < 5)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 4);
	par1 = (b & 0x7F);
	if (!(b & 0x80))
	{
		*buff_size = 5;
		goto done;
	}

	if(*buff_size < 6)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 5);
	par1 |= (b & 0x7F) << 7;
	if (!(b & 0x80))
	{
		*buff_size = 6;
		goto done;
	}

	if(*buff_size < 7)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 6);
	par1 |= (b & 0x7F) << 14;
	if (!(b & 0x80))
	{
		*buff_size = 7;
		goto done;
	}

	if(*buff_size < 8)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 7);
	par1 |= (b & 0x7F) << 21;
	if (!(b & 0x80))
	{
		*buff_size = 8;
		goto done;
	}

	//par2
	if(*buff_size < 9)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 8);
	par2 = (b & 0x7F);
	if (!(b & 0x80))
	{
		*buff_size = 9;
		goto done;
	}

	if(*buff_size < 10)
	{
		goto not_enough_byte_size;
	}
	b = *(uint8_t*)(buff_ptr + 9);
	par2 |= (b & 0x7F) << 7;
	if (!(b & 0x80))
	{
		*buff_size = 10;
		goto done;
	}
	return E_TLIBC_ERROR;
done:
	*result = ((uint64_t)par0) | ((uint64_t)par1 << 28 )| ((uint64_t)par2 << 56);
	return E_TLIBC_NOERROR;

not_enough_byte_size:
	return E_TLIBC_OUT_OF_MEMORY;
}

void tlibc_compact_reader_init(tlibc_compact_reader_t *self, const char *addr, uint32_t size)
{
	tlibc_abstract_reader_init(&self->super);

	self->super.read_char = tlibc_compact_read_char;
	self->super.read_double = tlibc_compact_read_double;
	self->super.read_int8 = tlibc_compact_read_int8;
	self->super.read_int16 = tlibc_compact_read_int16;
	self->super.read_int32 = tlibc_compact_read_int32;
	self->super.read_int64 = tlibc_compact_read_int64;
	self->super.read_uint8 = tlibc_compact_read_uint8;
	self->super.read_uint16 = tlibc_compact_read_uint16;
	self->super.read_uint32 = tlibc_compact_read_uint32;
	self->super.read_uint64 = tlibc_compact_read_uint64;
	self->super.read_string = tlibc_compact_read_string;



	self->addr = addr;
	self->size = size;
	self->offset = 0;
}

#define COMPACT_READER_CAPACITY(self) (self->size - self->offset)
#define COMPACT_READER_PTR(self) (self->addr + self->offset)


tlibc_error_code_t tlibc_compact_read_int8(tlibc_abstract_reader_t *super, int8_t *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	if(COMPACT_READER_CAPACITY(self) < sizeof(int8_t))
	{
		goto not_enough_bytebuff_size;
	}
	*val = *(int8_t*)COMPACT_READER_PTR(self);
	self->offset += sizeof(int8_t);

	return E_TLIBC_NOERROR;
not_enough_bytebuff_size:
	return E_TLIBC_OUT_OF_MEMORY;
}

tlibc_error_code_t tlibc_compact_read_int16(tlibc_abstract_reader_t *super, int16_t *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	uint16_t res;
	uint32_t buff_size = COMPACT_READER_CAPACITY(self);
	tlibc_error_code_t ret = tlibc_compact_varint16_decode(COMPACT_READER_PTR(self), &buff_size, &res);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	tlibc_little_to_host16(res);
	*val = tlibc_zigzag_decode16(res);
	self->offset +=buff_size;
done:
	return ret;
}

tlibc_error_code_t tlibc_compact_read_int32(tlibc_abstract_reader_t *super, int32_t *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	uint32_t res;
	uint32_t buff_size = COMPACT_READER_CAPACITY(self);
	tlibc_error_code_t ret = tlibc_compact_varint32_decode(COMPACT_READER_PTR(self), &buff_size, &res);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	tlibc_little_to_host32(res);
	*val = tlibc_zigzag_decode32(res);
	self->offset +=buff_size;
done:
	return ret;
}

tlibc_error_code_t tlibc_compact_read_int64(tlibc_abstract_reader_t *super, int64_t *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	uint64_t res;
	uint32_t buff_size = COMPACT_READER_CAPACITY(self);
	tlibc_error_code_t ret = tlibc_compact_varint64_decode(COMPACT_READER_PTR(self), &buff_size, &res);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	tlibc_little_to_host64(res);
	*val = tlibc_zigzag_decode64(res);
	self->offset +=buff_size;
done:
	return ret;
}

tlibc_error_code_t tlibc_compact_read_uint8(tlibc_abstract_reader_t *super, uint8_t *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	if(COMPACT_READER_CAPACITY(self) < sizeof(uint8_t))
	{
		goto not_enough_bytebuff_size;
	}
	*val = *(uint8_t*)COMPACT_READER_PTR(self);
	self->offset += sizeof(uint8_t);

	return E_TLIBC_NOERROR;
not_enough_bytebuff_size:
	return E_TLIBC_OUT_OF_MEMORY;
}

tlibc_error_code_t tlibc_compact_read_uint16(tlibc_abstract_reader_t *super, uint16_t *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	uint32_t buff_size = COMPACT_READER_CAPACITY(self);
	tlibc_error_code_t ret = tlibc_compact_varint16_decode(COMPACT_READER_PTR(self), &buff_size, val);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	tlibc_little_to_host16(*val);
	self->offset += buff_size;
done:
	return ret;
}

tlibc_error_code_t tlibc_compact_read_uint32(tlibc_abstract_reader_t *super, uint32_t *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	uint32_t buff_size = COMPACT_READER_CAPACITY(self);
	tlibc_error_code_t ret = tlibc_compact_varint32_decode(COMPACT_READER_PTR(self), &buff_size, val);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	tlibc_little_to_host32(*val);
	self->offset += buff_size;
done:
	return ret;
}

tlibc_error_code_t tlibc_compact_read_uint64(tlibc_abstract_reader_t *super, uint64_t *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	uint32_t buff_size = COMPACT_READER_CAPACITY(self);
	tlibc_error_code_t ret = tlibc_compact_varint64_decode(COMPACT_READER_PTR(self), &buff_size, val);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	tlibc_little_to_host64(*val);
	self->offset += buff_size;
done:
	return ret;
}

tlibc_error_code_t tlibc_compact_read_char(tlibc_abstract_reader_t *super, char *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	if(COMPACT_READER_CAPACITY(self) < sizeof(char))
	{
		goto not_enough_bytebuff_size;
	}
	*val = *(char*)COMPACT_READER_PTR(self);
	self->offset += sizeof(char);

	return E_TLIBC_NOERROR;
not_enough_bytebuff_size:
	return E_TLIBC_OUT_OF_MEMORY;
}


tlibc_error_code_t tlibc_compact_read_double(tlibc_abstract_reader_t *super, double *val)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	if(COMPACT_READER_CAPACITY(self) < sizeof(double))
	{
		goto not_enough_bytebuff_size;
	}
	*val = *(double*)COMPACT_READER_PTR(self);
	self->offset += sizeof(double);

	return E_TLIBC_NOERROR;
not_enough_bytebuff_size:
	return E_TLIBC_OUT_OF_MEMORY;
}


tlibc_error_code_t tlibc_compact_read_string(tlibc_abstract_reader_t *super, char* str, uint32_t str_length)
{
	tlibc_compact_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_reader_t, super);
	uint32_t str_len = 0;
	tlibc_error_code_t ret = E_TLIBC_NOERROR;

	for(; self->offset < self->size; )
	{
		char c;
		if(str_len >= str_length)
		{
			ret = E_TLIBC_OUT_OF_MEMORY;
			goto done;
		}
		c = (str[str_len++] = self->addr[self->offset++]);

		if(c == 0)
		{
			goto done;
		}
	}

	ret = E_TLIBC_OUT_OF_MEMORY;
done:
	return ret;
}

