#include "protocol/tlibc_abstract_reader.h"
#include "platform/tlibc_platform.h"
#include "core/tlibc_error_code.h"

#include <stdint.h>
#include <stdio.h>

void tlibc_abstract_reader_init(tlibc_abstract_reader_t *self)
{
	self->enable_name = FALSE;
	self->name[0] = 0;
	self->name_ptr = self->name;

	self->read_struct_begin = NULL;
	self->read_struct_end = NULL;
	self->read_enum_begin = NULL;
	self->read_enum_end = NULL;
	self->read_union_begin = NULL;
	self->read_union_end = NULL;
	self->read_vector_begin = NULL;
	self->read_vector_end = NULL;
	self->read_field_begin = NULL;
	self->read_field_end = NULL;
	self->read_vector_element_begin = NULL;
	self->read_vector_element_end = NULL;

	self->read_int8 = NULL;
	self->read_int16 = NULL;
	self->read_int32 = NULL;
	self->read_int64 = NULL;
	self->read_uint8 = NULL;
	self->read_uint16 = NULL;
	self->read_uint32 = NULL;
	self->read_uint64 = NULL;
	self->read_char = NULL;
	self->read_double = NULL;
	self->read_string = NULL;
}

tlibc_error_code_t tlibc_read_struct_begin(tlibc_abstract_reader_t *self, const char *struct_name)
{
	if(self->read_struct_begin == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_struct_begin(self, struct_name);
}

tlibc_error_code_t tlibc_read_struct_end(tlibc_abstract_reader_t *self, const char *struct_name)
{
	if(self->read_struct_end == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_struct_end(self, struct_name);
}


tlibc_error_code_t tlibc_read_union_begin(tlibc_abstract_reader_t *self, const char *union_name)
{
	if(self->read_union_begin == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_union_begin(self, union_name);
}

tlibc_error_code_t tlibc_read_union_end(tlibc_abstract_reader_t *self, const char *union_name)
{
	if(self->read_union_end == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_union_end(self, union_name);
}


tlibc_error_code_t tlibc_read_enum_begin(tlibc_abstract_reader_t *self, const char *enum_name)
{
	if(self->read_enum_begin == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_enum_begin(self, enum_name);
}

tlibc_error_code_t tlibc_read_enum_end(tlibc_abstract_reader_t *self, const char *enum_name)
{
	if(self->read_enum_end == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_enum_end(self, enum_name);
}

static tlibc_error_code_t add_name(tlibc_abstract_reader_t *self, const char *name)
{
	if(self->enable_name)
	{
		int len = snprintf(self->name_ptr, TLIBC_READER_NAME_LENGTH - (self->name_ptr - self->name)
			, ".%s", name);

		if(len < 0)
		{
			return E_TLIBC_ERROR;
		}

		self->name_ptr += len;
	}

	return E_TLIBC_NOERROR;
}

static tlibc_error_code_t del_name(tlibc_abstract_reader_t *self, const char *name)
{
	if(self->enable_name)
	{	
		char curr_name[TLIBC_READER_NAME_LENGTH];	
		int len = snprintf(curr_name, TLIBC_READER_NAME_LENGTH - (self->name_ptr - self->name)
			, ".%s", name);

		if(len < 0)
		{
			return E_TLIBC_ERROR;
		}
		self->name_ptr -= len;
	}
	return E_TLIBC_NOERROR;
}

tlibc_error_code_t tlibc_read_vector_begin(tlibc_abstract_reader_t *self, const char* vec_name)
{
	tlibc_error_code_t ret;
	ret = add_name(self, vec_name);
	if(ret != E_TLIBC_NOERROR)
	{
		return ret;
	}
	if(self->read_vector_begin == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_vector_begin(self, vec_name);
}

tlibc_error_code_t tlibc_read_vector_end(tlibc_abstract_reader_t *self, const char* vec_name)
{
	tlibc_error_code_t ret;
	ret = del_name(self, vec_name);
	if(ret != E_TLIBC_NOERROR)
	{
		return ret;
	}
	if(self->read_vector_end == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_vector_end(self, vec_name);
}



tlibc_error_code_t tlibc_read_field_begin(tlibc_abstract_reader_t *self, const char *var_name)
{
	tlibc_error_code_t ret;
	ret = add_name(self, var_name);
	if(ret != E_TLIBC_NOERROR)
	{
		return ret;
	}
	if(self->read_field_begin == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_field_begin(self, var_name);
}

tlibc_error_code_t tlibc_read_field_end(tlibc_abstract_reader_t *self, const char *var_name)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;

	if(self->read_field_end != NULL)
	{
		ret = self->read_field_end(self, var_name);
		if(ret != E_TLIBC_NOERROR)
		{
			return ret;
		}
	}	
	ret = del_name(self, var_name);
	return ret;
}

tlibc_error_code_t tlibc_read_vector_element_begin(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index)
{
	if(self->enable_name)
	{
		int len = snprintf(self->name_ptr, TLIBC_READER_NAME_LENGTH - (self->name_ptr - self->name)
				, "[%u]", index);

		if(len < 0)
		{
			return E_TLIBC_ERROR;
		}
		self->name_ptr += len;
	}
	
	if(self->read_vector_element_begin == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_vector_element_begin(self, var_name, index);
}

tlibc_error_code_t tlibc_read_vector_element_end(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;

	if(self->read_vector_element_end != NULL)
	{
		ret = self->read_vector_element_end(self, var_name, index);
	}	

	if(self->enable_name)
	{		
		char curr_name[TLIBC_READER_NAME_LENGTH];
		int len = snprintf(curr_name, TLIBC_READER_NAME_LENGTH, "[%u]", index);
		
		if(len < 0)
		{
			return E_TLIBC_ERROR;
		}
		self->name_ptr -= len;
	}
	return ret;
}

tlibc_error_code_t tlibc_read_int8(tlibc_abstract_reader_t *self, int8_t *val)
{
	if(self->read_int8 == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_int8(self, val);
}

tlibc_error_code_t tlibc_read_int16(tlibc_abstract_reader_t *self, int16_t *val)
{
	if(self->read_int16 == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_int16(self, val);
}

tlibc_error_code_t tlibc_read_int32(tlibc_abstract_reader_t *self, int32_t *val)
{
	if(self->read_int32 == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_int32(self, val);
}

tlibc_error_code_t tlibc_read_int64(tlibc_abstract_reader_t *self, int64_t *val)
{
	if(self->read_int64 == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_int64(self, val);
}

tlibc_error_code_t tlibc_read_uint8(tlibc_abstract_reader_t *self, uint8_t *val)
{
	if(self->read_uint8 == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_uint8(self, val);
}

tlibc_error_code_t tlibc_read_uint16(tlibc_abstract_reader_t *self, uint16_t *val)
{
	if(self->read_uint16 == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_uint16(self, val);
}

tlibc_error_code_t tlibc_read_uint32(tlibc_abstract_reader_t *self, uint32_t *val)
{
	if(self->read_uint32 == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_uint32(self, val);
}

tlibc_error_code_t tlibc_read_uint64(tlibc_abstract_reader_t *self, uint64_t *val)
{
	if(self->read_uint64 == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_uint64(self, val);
}

tlibc_error_code_t tlibc_read_char(tlibc_abstract_reader_t *self, char *val)
{
	if(self->read_char == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_char(self, val);
}

tlibc_error_code_t tlibc_read_double(tlibc_abstract_reader_t *self, double *val)
{
	if(self->read_double == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_double(self, val);
}

tlibc_error_code_t tlibc_read_string(tlibc_abstract_reader_t *self, char* str, uint32_t str_length)
{
	if(self->read_string == NULL)
	{
		return E_TLIBC_NOERROR;
	}
	return self->read_string(self, str, str_length);
}

