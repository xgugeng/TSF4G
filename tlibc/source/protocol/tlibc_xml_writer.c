#include "protocol/tlibc_xml_writer.h"
#include "core/tlibc_error_code.h"
#include "core/tlibc_util.h"
#include "protocol/tlibc_abstract_writer.h"


#include <string.h>
#include <assert.h>
#include <stdio.h>


void tlibc_xml_writer_init(tlibc_xml_writer_t *self, char *start, char*limit)
{
	tlibc_abstract_writer_init(&self->super);


	self->cur = start;
	self->start = start;
	self->limit = limit;
	self->count = 0;

	self->super.write_struct_begin = tlibc_xml_write_struct_begin;
	self->super.write_struct_end = tlibc_xml_write_struct_end;
	self->super.write_union_begin = tlibc_xml_write_struct_begin;
	self->super.write_union_end = tlibc_xml_write_struct_end;
	self->super.write_enum_begin = tlibc_xml_write_enum_begin;

	self->super.write_vector_begin = tlibc_xml_write_vector_begin;
	self->super.write_vector_end = tlibc_xml_write_vector_end;
	self->super.write_field_begin = tlibc_xml_write_field_begin;
	self->super.write_field_end = tlibc_xml_write_field_end;
	self->super.write_vector_element_begin = tlibc_xml_write_vector_element_begin;
	self->super.write_vector_element_end = tlibc_xml_write_vector_element_end;
	
	self->super.write_int8 = tlibc_xml_write_int8;
	self->super.write_int16 = tlibc_xml_write_int16;
	self->super.write_int32 = tlibc_xml_write_int32;
	self->super.write_int64 = tlibc_xml_write_int64;

	self->super.write_uint8 = tlibc_xml_write_uint8;
	self->super.write_uint16 = tlibc_xml_write_uint16;
	self->super.write_uint32 = tlibc_xml_write_uint32;
	self->super.write_uint64 = tlibc_xml_write_uint64;


	self->super.write_double = tlibc_xml_write_double;
	self->super.write_string = tlibc_xml_write_string;
	self->super.write_char = tlibc_xml_write_char;


	self->skip_uint32_field_once = FALSE;
	self->ignore_int32_once = FALSE;
}

tlibc_error_code_t tlibc_xml_write_struct_begin(tlibc_abstract_writer_t *super, const char *struct_name)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);	
	size_t slen, len;

	if(self->count == 0)
	{
		slen = strlen(struct_name);
		len = slen + 1 + 1;
		if((size_t)(self->limit - self->cur) < len)
		{
			ret = E_TLIBC_OUT_OF_MEMORY;
			goto done;
		}
		*(self->cur++) = '<';
		memcpy(self->cur, struct_name, slen);
		self->cur += slen;
		*(self->cur++) = '>';
	}
	
done:
	return ret;
}

tlibc_error_code_t tlibc_xml_write_struct_end(tlibc_abstract_writer_t *super, const char *struct_name)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);	
	size_t slen, len;
	if(self->count == 0)
	{
		slen = strlen(struct_name);
		len = slen + 3;
		if((size_t)(self->limit - self->cur) < len)
		{
			ret = E_TLIBC_OUT_OF_MEMORY;
			goto done;
		}
		*(self->cur++) = '<';
		*(self->cur++) = '/';
		memcpy(self->cur, struct_name, slen);
		self->cur += slen;
		*(self->cur++) = '>';
	}
done:
	return ret;
}

tlibc_error_code_t tlibc_xml_write_enum_begin(tlibc_abstract_writer_t *super, const char *enum_name)
{
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);
	TLIBC_UNUSED(enum_name);
	self->ignore_int32_once = TRUE;
	return E_TLIBC_NOERROR;
}

tlibc_error_code_t tlibc_xml_write_vector_begin(tlibc_abstract_writer_t *super, const char* vec_name)
{
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);	
	tlibc_error_code_t ret = tlibc_xml_write_field_begin(super, vec_name);
	self->skip_uint32_field_once = TRUE;
	return ret;
}

tlibc_error_code_t tlibc_xml_write_vector_end(tlibc_abstract_writer_t *super, const char* vec_name)
{
	return tlibc_xml_write_field_end(super, vec_name);
}

tlibc_error_code_t tlibc_xml_write_field_begin(tlibc_abstract_writer_t *super, const char *var_name)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);
	size_t len, slen;
	if(self->skip_uint32_field_once)
	{
		goto done;
	}
	++self->count;

	slen = strlen(var_name);
	len = 1 + slen + 1;
	
	if((size_t)(self->limit - self->cur) < len)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}
	
	*(self->cur++) = '<';
	memcpy(self->cur, var_name, slen);
	self->cur+=slen;
	*(self->cur++) = '>';
done:
	return E_TLIBC_NOERROR;
}

tlibc_error_code_t tlibc_xml_write_field_end(tlibc_abstract_writer_t *super, const char *var_name)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);	
	size_t len, slen;
	if(self->skip_uint32_field_once)
	{
		self->skip_uint32_field_once = FALSE;
		goto done;
	}

	slen = strlen(var_name);
	len = 1 + 1 + slen + 1;
	
	if((size_t)(self->limit - self->cur) < len)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}
	
	--(self->count);
	*(self->cur++) = '<';
	*(self->cur++) = '/';
	memcpy(self->cur, var_name, slen);
	self->cur += slen;
	*(self->cur++) = '>';

done:
	return E_TLIBC_NOERROR;
}

tlibc_error_code_t tlibc_xml_write_vector_element_begin(tlibc_abstract_writer_t *super, const char *var_name, uint32_t index)
{	
	TLIBC_UNUSED(var_name);
	TLIBC_UNUSED(index);
	return tlibc_xml_write_field_begin(super, "element");
}

tlibc_error_code_t tlibc_xml_write_vector_element_end(tlibc_abstract_writer_t *super, const char *var_name, uint32_t index)
{
	TLIBC_UNUSED(var_name);
	TLIBC_UNUSED(index);
	return tlibc_xml_write_field_end(super, "element");
}
#define TLIBC_XML_VALUE_LEN 128

tlibc_error_code_t tlibc_xml_write_double(tlibc_abstract_writer_t *super, const double *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);
	char str[TLIBC_XML_VALUE_LEN];
	size_t len;
	len = snprintf(str, TLIBC_XML_VALUE_LEN, "%lf", *val);
	if((size_t)(self->limit - self->cur) < len)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}
	memcpy(self->cur, str, len);
	self->cur += len;
done:
	return ret;
}

tlibc_error_code_t tlibc_xml_write_int8(tlibc_abstract_writer_t *super, const int8_t *val)
{
	int64_t v = *val;
	return tlibc_xml_write_int64(super, &v);
}

tlibc_error_code_t tlibc_xml_write_int16(tlibc_abstract_writer_t *super, const int16_t *val)
{
	int64_t v = *val;
	return tlibc_xml_write_int64(super, &v);
}

tlibc_error_code_t tlibc_xml_write_int32(tlibc_abstract_writer_t *super, const int32_t *val)
{
	int64_t v;
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);
	if(self->ignore_int32_once)
	{
		self->ignore_int32_once = FALSE;
		ret = E_TLIBC_PLEASE_READ_ENUM_NAME;
		goto done;
	}
	v = *val;
	ret = tlibc_xml_write_int64(super, &v);
done:
	return ret;
}

tlibc_error_code_t tlibc_xml_write_int64(tlibc_abstract_writer_t *super, const int64_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);
	char str[TLIBC_XML_VALUE_LEN];
	size_t len;
	len = snprintf(str, TLIBC_XML_VALUE_LEN, "%"PRIi64, *val);
	if((size_t)(self->limit - self->cur) < len)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}
	memcpy(self->cur, str, len);
	self->cur += len;
done:
	return ret;
}


tlibc_error_code_t tlibc_xml_write_uint8(tlibc_abstract_writer_t *super, const uint8_t *val)
{
	uint64_t v = *val;
	return tlibc_xml_write_uint64(super, &v);
}

tlibc_error_code_t tlibc_xml_write_uint16(tlibc_abstract_writer_t *super, const uint16_t *val)
{
	uint64_t v = *val;
	
	return tlibc_xml_write_uint64(super, &v);
}

tlibc_error_code_t tlibc_xml_write_uint32(tlibc_abstract_writer_t *super, const uint32_t *val)
{
	uint64_t v = *val;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);
	if(self->skip_uint32_field_once)
	{
		goto done;
	}
	return tlibc_xml_write_uint64(super, &v);
done:
	return E_TLIBC_NOERROR;
}

tlibc_error_code_t tlibc_xml_write_uint64(tlibc_abstract_writer_t *super, const uint64_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);
	char str[TLIBC_XML_VALUE_LEN];
	size_t len;
	len = snprintf(str, TLIBC_XML_VALUE_LEN, "%"PRIu64, *val);
	if((size_t)(self->limit - self->cur) < len)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}
	memcpy(self->cur, str, len);
	self->cur += len;
done:
	return ret;
}

static char* write_char(char *cur, char *limit, char c)
{
	const char *str = NULL;
	size_t len = 0;
	switch (c)
	{
	case '<':
		str = "&lt";
		len = 3;
		break;
	case '>':
		str = "&gt";
		len = 3;
		break;
	case '&':
		str = "&amp";
		len = 4;
		break;
	case '\'':
		str = "&apos";
		len = 6;
		break;
	case '\"':
		str = "&quot";
		len = 5;
		break;
	default:
		str = &c;
		len = 1;
		break;
	}

	if((size_t)(limit - cur) < len)
	{
		return NULL;
	}
	memcpy(cur, str, len);
	return cur + len;
}

tlibc_error_code_t tlibc_xml_write_string(tlibc_abstract_writer_t *super, const char* str, uint32_t str_length)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);
	const char *i;	
	TLIBC_UNUSED(str_length);
	for(i = str; *i ; ++i)
	{
		char *next = write_char(self->cur, self->limit, *i);
		if(next == NULL)
		{
			ret = E_TLIBC_OUT_OF_MEMORY;
			goto done;
		}
		self->cur = next;
	}
done:
	return ret;
}

tlibc_error_code_t tlibc_xml_write_char(tlibc_abstract_writer_t *super, const char *val)
{
	tlibc_xml_writer_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_writer_t, super);
	char *next = write_char(self->cur, self->limit, *val);
	if(next == NULL)
	{
		return E_TLIBC_OUT_OF_MEMORY;
	}

	self->cur = next;
	return E_TLIBC_NOERROR;
}
