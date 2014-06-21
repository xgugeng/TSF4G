#include "protocol/tlibc_compact_writer.h"
#include "platform/tlibc_platform.h"
#include "core/tlibc_util.h"
#include "protocol/tlibc_abstract_writer.h"
#include "core/tlibc_error_code.h"

#include <string.h>
#include <assert.h>


static tlibc_error_code_t tlibc_compact_varint16_encode(uint16_t n, char *buff_ptr, uint32_t *buff_size)
{
	if(*buff_size < 1)
	{
		goto not_enough_byte_size;
	}
	buff_ptr[0] = (char)(n | 0x80);
	if (n >= (1 << 7))
	{
		if(*buff_size < 2)
		{
			goto not_enough_byte_size;
		}
		buff_ptr[1] = (char)((n >>  7) | 0x80);
		if (n >= (1 << 14))
		{
			if(*buff_size < 3)
			{
				goto not_enough_byte_size;
			}
			buff_ptr[2] = (char)(n >> 14);
			*buff_size = 3;
			goto done;
		}
		else
		{
			buff_ptr[1] &= 0x7F;
			*buff_size = 2;
			goto done;
		}
	}
	else
	{
		buff_ptr[0] &= 0x7F;
		*buff_size = 1;
		goto done;
	}

done:
	return E_TLIBC_NOERROR;

not_enough_byte_size:
	return E_TLIBC_OUT_OF_MEMORY;
}



static tlibc_error_code_t tlibc_compact_varint32_encode(uint32_t n, char *buff_ptr, uint32_t *buff_size)
{
	if(*buff_size < 1)
	{
		goto not_enough_byte_size;
	}
	buff_ptr[0] = (char)(n | 0x80);
	if (n >= (1 << 7))
	{
		if(*buff_size < 2)
		{
			goto not_enough_byte_size;
		}
		buff_ptr[1] = (char)((n >>  7) | 0x80);
		if (n >= (1 << 14))
		{
			if(*buff_size < 3)
			{
				goto not_enough_byte_size;
			}
			buff_ptr[2] = (char)((n >> 14) | 0x80);

			if (n >= (1 << 21))
			{
				if(*buff_size < 4)
				{
					goto not_enough_byte_size;
				}
				buff_ptr[3] = (char)((n >> 21) | 0x80);

				if (n >= (1 << 28))
				{
					if(*buff_size < 5)
					{
						goto not_enough_byte_size;
					}
					buff_ptr[4] = (char)(n >> 28);
					*buff_size = 5;
					goto done;
				}
				else
				{
					buff_ptr[3] &= 0x7F;
					*buff_size = 4;
					goto done;
				}
			}
			else
			{
				buff_ptr[2] &= 0x7F;
				*buff_size = 3;
				goto done;
			}
		}
		else
		{
			buff_ptr[1] &= 0x7F;
			*buff_size = 2;
			goto done;
		}
	}
	else
	{
		buff_ptr[0] &= 0x7F;
		*buff_size = 1;
		goto done;
	}

done:
	return E_TLIBC_NOERROR;

not_enough_byte_size:
	return E_TLIBC_OUT_OF_MEMORY;
}

static tlibc_error_code_t tlibc_compact_varint64_encode(uint64_t n, char *buff_ptr, uint32_t *buff_size)
{
	uint32_t part0 = (uint32_t)(n);
	uint32_t part1 = (uint32_t)(n >> 28);
	uint32_t part2 = (uint32_t)(n >> 56);

	int32_t size;

	if (part2 == 0)
	{
		if (part1 == 0)
		{
			if (part0 < (1 << 14))
			{
				if (part0 < (1 << 7))
				{
					size = 1;
					if(*buff_size < 1)
					{
						goto not_enough_byte_size;
					}
					*buff_size = 1;

					goto size1;
				}
				else
				{
					size = 2;
					if(*buff_size < 2)
					{
						goto not_enough_byte_size;
					}
					*buff_size = 2;

					goto size2;
				}
			}
			else
			{
				if (part0 < (1 << 21))
				{
					size = 3;
					if(*buff_size < 3)
					{
						goto not_enough_byte_size;
					}
					*buff_size = 3;

					goto size3;
				}
				else
				{
					size = 4;
					if(*buff_size < 4)
					{
						goto not_enough_byte_size;
					}
					*buff_size = 4;

					goto size4;
				}
			}
		}
		else
		{
			if (part1 < (1 << 14))
			{
				if (part1 < (1 << 7))
				{
					size = 5;
					if(*buff_size < 5)
					{
						goto not_enough_byte_size;
					}
					*buff_size = 5;
					goto size5;
				}
				else
				{
					size = 6;
					if(*buff_size < 6)
					{
						goto not_enough_byte_size;
					}
					*buff_size = 6;
					goto size6;
				}
			}
			else
			{
				if (part1 < (1 << 21))
				{
					size = 7;
					if(*buff_size < 7)
					{
						goto not_enough_byte_size;
					}
					*buff_size = 7;
					goto size7;
				}
				else
				{
					size = 8;
					if(*buff_size < 8)
					{
						goto not_enough_byte_size;
					}
					*buff_size = 8;
					goto size8;
				}
			}
		}
	}
	else
	{
		if (part2 < (1 << 7))
		{
			size = 9;
			if(*buff_size < 9)
			{
				goto not_enough_byte_size;
			}
			*buff_size = 9;
			goto size9;
		}
		else
		{
			size = 10;
			if(*buff_size < 10)
			{
				goto not_enough_byte_size;
			}
			*buff_size = 10;
			goto size10;
		}
	}


size10:
	buff_ptr[9] = (char)((part2 >>  7) | 0x80);
size9:
	buff_ptr[8] = (char)((part2      ) | 0x80);
size8:
	buff_ptr[7] = (char)((part1 >> 21) | 0x80);
size7:
	buff_ptr[6] = (char)((part1 >> 14) | 0x80);
size6:
	buff_ptr[5] = (char)((part1 >>  7) | 0x80);
size5:
	buff_ptr[4] = (char)((part1      ) | 0x80);
size4:
	buff_ptr[3] = (char)((part0 >> 21) | 0x80);
size3:
	buff_ptr[2] = (char)((part0 >> 14) | 0x80);
size2:
	buff_ptr[1] = (char)((part0 >>  7) | 0x80);
size1:
	buff_ptr[0] = (char)((part0      ) | 0x80);

	buff_ptr[size-1] &= 0x7F;
	return E_TLIBC_NOERROR;

not_enough_byte_size:
	return E_TLIBC_OUT_OF_MEMORY;
}


void tlibc_compact_writer_init(tlibc_compact_writer_t *self, char *addr, uint32_t size)
{
	tlibc_abstract_writer_init(&self->super);

	self->super.write_char = tlibc_compact_write_char;
	self->super.write_double = tlibc_compact_write_double;
	self->super.write_int8 = tlibc_compact_write_int8;
	self->super.write_int16 = tlibc_compact_write_int16;
	self->super.write_int32 = tlibc_compact_write_int32;
	self->super.write_int64 = tlibc_compact_write_int64;
	self->super.write_uint8 = tlibc_compact_write_uint8;
	self->super.write_uint16 = tlibc_compact_write_uint16;
	self->super.write_uint32 = tlibc_compact_write_uint32;
	self->super.write_uint64 = tlibc_compact_write_uint64;
	self->super.write_string = tlibc_compact_write_string;

	self->addr = addr;
	self->size = size;
	self->offset = 0;
}

#define COMPACT_WRITER_CAPACITY(self) (self->size - self->offset)
#define COMPACT_WRITER_PTR(self) (self->addr + self->offset)

tlibc_error_code_t tlibc_compact_write_int8(tlibc_abstract_writer_t *super, const int8_t *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	if(COMPACT_WRITER_CAPACITY(self) < sizeof(int8_t))
	{
		goto not_enough_bytebuff_size;
	}
	*(int8_t*)COMPACT_WRITER_PTR(self) = *val;
	self->offset += sizeof(int8_t);

	return E_TLIBC_NOERROR;
not_enough_bytebuff_size:
	return E_TLIBC_OUT_OF_MEMORY;
}

tlibc_error_code_t tlibc_compact_write_int16(tlibc_abstract_writer_t *super, const int16_t *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	uint32_t buff_size = COMPACT_WRITER_CAPACITY(self);
	int16_t v = tlibc_zigzag_encode16(*val);
	tlibc_error_code_t ret;
	tlibc_host16_to_little(v);
	ret = tlibc_compact_varint16_encode(v, COMPACT_WRITER_PTR(self), &buff_size);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	self->offset += buff_size;

done:
	return ret;
}

tlibc_error_code_t tlibc_compact_write_int32(tlibc_abstract_writer_t *super, const int32_t *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	uint32_t buff_size = COMPACT_WRITER_CAPACITY(self);
	int32_t v = tlibc_zigzag_encode16(*val);
	tlibc_error_code_t ret;
	tlibc_host32_to_little(v);
	ret = tlibc_compact_varint32_encode(v, COMPACT_WRITER_PTR(self), &buff_size);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	self->offset += buff_size;

done:
	return ret;
}

tlibc_error_code_t tlibc_compact_write_int64(tlibc_abstract_writer_t *super, const int64_t *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	uint32_t buff_size = COMPACT_WRITER_CAPACITY(self);
	int64_t v = tlibc_zigzag_encode64(*val);
	tlibc_error_code_t ret;
	tlibc_host64_to_little(v);
	ret = tlibc_compact_varint64_encode(v, COMPACT_WRITER_PTR(self), &buff_size);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	self->offset += buff_size;

done:
	return ret;
}


tlibc_error_code_t tlibc_compact_write_uint8(tlibc_abstract_writer_t *super, const uint8_t *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	if(COMPACT_WRITER_CAPACITY(self) < sizeof(uint8_t))
	{
		goto not_enough_bytebuff_size;
	}
	*(uint8_t*)COMPACT_WRITER_PTR(self) = *val;
	self->offset += sizeof(uint8_t);

	return E_TLIBC_NOERROR;
not_enough_bytebuff_size:
	return E_TLIBC_OUT_OF_MEMORY;
}

tlibc_error_code_t tlibc_compact_write_uint16(tlibc_abstract_writer_t *super, const uint16_t *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	uint32_t buff_size = COMPACT_WRITER_CAPACITY(self);
	uint16_t v = *val;
	tlibc_error_code_t ret;
	tlibc_host16_to_little(v);
	ret = tlibc_compact_varint16_encode(v, COMPACT_WRITER_PTR(self), &buff_size);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	self->offset += buff_size;

done:
	return ret;
}

tlibc_error_code_t tlibc_compact_write_uint32(tlibc_abstract_writer_t *super, const uint32_t *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	uint32_t buff_size = COMPACT_WRITER_CAPACITY(self);
	uint32_t v = *val;
	tlibc_error_code_t ret;
	tlibc_host32_to_little(v);
	ret = tlibc_compact_varint32_encode(v, COMPACT_WRITER_PTR(self), &buff_size);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	self->offset += buff_size;

done:
	return ret;
}

tlibc_error_code_t tlibc_compact_write_uint64(tlibc_abstract_writer_t *super, const uint64_t *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	uint32_t buff_size = COMPACT_WRITER_CAPACITY(self);
	uint64_t v = *val;
	tlibc_error_code_t ret;
	tlibc_host64_to_little(v);
	ret = tlibc_compact_varint64_encode(v, COMPACT_WRITER_PTR(self), &buff_size);
	if(ret != E_TLIBC_NOERROR)
	{
		goto done;
	}
	self->offset += buff_size;

done:
	return ret;
}

tlibc_error_code_t tlibc_compact_write_char(tlibc_abstract_writer_t *super, const char *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	if(COMPACT_WRITER_CAPACITY(self) < sizeof(char))
	{
		goto not_enough_bytebuff_size;
	}
	*(char*)COMPACT_WRITER_PTR(self) = *val;
	self->offset += sizeof(char);

	return E_TLIBC_NOERROR;
not_enough_bytebuff_size:
	return E_TLIBC_OUT_OF_MEMORY;
}

tlibc_error_code_t tlibc_compact_write_double(tlibc_abstract_writer_t *super, const double *val)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	if(COMPACT_WRITER_CAPACITY(self) < sizeof(double))
	{
		goto not_enough_bytebuff_size;
	}
	*(double*)COMPACT_WRITER_PTR(self) = *val;
	self->offset += sizeof(double);

	return E_TLIBC_NOERROR;
not_enough_bytebuff_size:
	return E_TLIBC_OUT_OF_MEMORY;
}

tlibc_error_code_t tlibc_compact_write_string(tlibc_abstract_writer_t *super, const char* str, uint32_t str_length)
{
	tlibc_compact_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_compact_writer_t, super);
	uint32_t str_len = 0;
	tlibc_error_code_t ret= E_TLIBC_NOERROR;
	TLIBC_UNUSED(str_length);

	for(; self->offset < self->size; )
	{
		char c = (self->addr[self->offset++] = str[str_len++]);

		if(c == 0)
		{
			goto done;
		}
	}
	ret = E_TLIBC_OUT_OF_MEMORY;
done:
	return ret;
}
