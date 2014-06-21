#include "protocol/tlibc_mybind_writer.h"
#include "platform/tlibc_platform.h"
#include "core/tlibc_util.h"
#include "protocol/tlibc_abstract_writer.h"
#include "core/tlibc_error_code.h"

#include <string.h>
#include <assert.h>
#include <stdint.h>


void tlibc_mybind_writer_init(tlibc_mybind_writer_t *self, MYSQL_BIND *bind_vec, uint32_t bind_vec_num)
{
	tlibc_abstract_writer_init(&self->super);

	self->super.write_int8 = tlibc_mybind_write_int8;
	self->super.write_int16 = tlibc_mybind_write_int16;
	self->super.write_int32 = tlibc_mybind_write_int32;
	self->super.write_int64 = tlibc_mybind_write_int64;
	self->super.write_uint8 = tlibc_mybind_write_uint8;
	self->super.write_uint16 = tlibc_mybind_write_uint16;
	self->super.write_uint32 = tlibc_mybind_write_uint32;
	self->super.write_uint64 = tlibc_mybind_write_uint64;
	self->super.write_double = tlibc_mybind_write_double;
	self->super.write_char = tlibc_mybind_write_char;
	self->super.write_string = tlibc_mybind_write_string;
	

	self->bind_vec = bind_vec;
	self->bind_vec_num = bind_vec_num;
	self->idx = 0;
}

tlibc_error_code_t tlibc_mybind_write_int8(tlibc_abstract_writer_t *super, const int8_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);
	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_TINY;
	self->bind_vec[self->idx].buffer = (void*)val;
	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}

tlibc_error_code_t tlibc_mybind_write_int16(tlibc_abstract_writer_t *super, const int16_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);
	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_SHORT;
	self->bind_vec[self->idx].buffer = (void*)val;
	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}

tlibc_error_code_t tlibc_mybind_write_int32(tlibc_abstract_writer_t *super, const int32_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);

	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_LONG;
	self->bind_vec[self->idx].buffer = (void*)val;
	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}

tlibc_error_code_t tlibc_mybind_write_int64(tlibc_abstract_writer_t *super, const int64_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);
	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_LONGLONG;
	self->bind_vec[self->idx].buffer = (void*)val;
	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}


tlibc_error_code_t tlibc_mybind_write_uint8(tlibc_abstract_writer_t *super, const uint8_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);
	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_TINY;
	self->bind_vec[self->idx].buffer = (void*)val;
	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}

tlibc_error_code_t tlibc_mybind_write_uint16(tlibc_abstract_writer_t *super, const uint16_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);
	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_SHORT;
	self->bind_vec[self->idx].buffer = (void*)val;
	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}

tlibc_error_code_t tlibc_mybind_write_uint32(tlibc_abstract_writer_t *super, const uint32_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);
	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_LONG;
	self->bind_vec[self->idx].buffer = (void*)val;
	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}

tlibc_error_code_t tlibc_mybind_write_uint64(tlibc_abstract_writer_t *super, const uint64_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);
	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_LONGLONG;
	self->bind_vec[self->idx].buffer = (void*)val;
	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}


tlibc_error_code_t tlibc_mybind_write_double(tlibc_abstract_writer_t *super, const double *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);
	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_DOUBLE;
	self->bind_vec[self->idx].buffer = (void*)val;
	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}

tlibc_error_code_t tlibc_mybind_write_char(tlibc_abstract_writer_t *super, const char *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);

	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_STRING;
	self->bind_vec[self->idx].buffer = (void*)val;
	self->bind_vec[self->idx].buffer_length = 1;

	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}


tlibc_error_code_t tlibc_mybind_write_string(tlibc_abstract_writer_t *super, const char* str, uint32_t str_length)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_mybind_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_mybind_writer_t, super);
	TLIBC_UNUSED(str_length);

	if(self->idx >= self->bind_vec_num)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}

	self->bind_vec[self->idx].buffer_type = MYSQL_TYPE_STRING;
	self->bind_vec[self->idx].buffer = (void*)str;
	self->bind_vec[self->idx].buffer_length = str_length;

	++(self->idx);

	return E_TLIBC_NOERROR;
done:
	return ret;
}
