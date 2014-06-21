#include "protocol/tlibc_xml_reader.h"
#include "core/tlibc_util.h"
#include "protocol/tlibc_abstract_reader.h"
#include "core/tlibc_error_code.h"
#include "tlibc_xml_reader_l.h"

#include "tlibc_xml_reader_scanner.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

tlibc_error_code_t tlibc_xml_reader_push_file(tlibc_xml_reader_t *self, const char *file_name)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	FILE* fin;
	size_t file_size;
	char c;
	char *start, *curr, *limit;
	size_t i;

	for(i = 0; i < self->scanner_context_stack_num ; ++i)
	{
		if(self->scanner_context_stack[i].filecontent_ptr)
		{
			if(strcmp(self->scanner_context_stack[i].yylloc.file_name, file_name) == 0)
			{
				ret = E_TLIBC_FILE_IS_ALREADY_ON_THE_STACK;
				goto done;
			}
		}		
	}

	

	fin = fopen(file_name, "rb");
	if(fin == NULL)
	{
		size_t i;
		char fn[TLIBC_MAX_PATH_LENGTH];
		for(i = 0; i < self->include_num; ++i)
		{
			snprintf(fn, TLIBC_MAX_PATH_LENGTH, "%s/%s", self->include[i], file_name);
			fin = fopen(fn, "rb");
			if(fin != NULL)
			{
				break;
			}
		}		
	}
	if(fin == NULL)
	{
		ret = E_TLIBC_NOT_FOUND;
		goto done;
	}	

	fseek(fin, 0, SEEK_END);
	file_size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	start = (char*)malloc(file_size);
	if(start == NULL)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;		
		goto free_file;
	}
	curr = start;
	limit = start + file_size;

	while((c = (char)fgetc(fin)) != EOF)
	{
		if(curr == limit)
		{
			ret = E_TLIBC_OUT_OF_MEMORY;
			goto free_buff;
		}
		*curr = c;
		++curr;
	}
	
	ret = tlibc_xml_reader_push_buff(self, start, limit);
	if(ret != E_TLIBC_NOERROR)
	{
		goto free_buff;
	}
	fclose(fin);

	strncpy(self->scanner_context_stack[self->scanner_context_stack_num - 1].yylloc.file_name, file_name, TLIBC_MAX_PATH_LENGTH - 1);
	self->scanner_context_stack[self->scanner_context_stack_num - 1].filecontent_ptr = start;
	
	return E_TLIBC_NOERROR;
free_buff:
	free(start);
free_file:
	fclose(fin);
done:
	return ret;
}

tlibc_error_code_t tlibc_xml_reader_push_buff(tlibc_xml_reader_t *self, const char *xml_start, const char* xml_limit)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_reader_scanner_context_t *scanner = NULL;
	if(self->scanner_context_stack_num >= TLIBC_XML_MAX_DEEP)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto ERROR_RET;
	}
	scanner = &self->scanner_context_stack[self->scanner_context_stack_num];
	scanner->yy_state = yycINITIAL;
	scanner->yy_start = xml_start;
	scanner->yy_limit = xml_limit;
	scanner->yy_cursor = scanner->yy_start;	
	scanner->yy_marker = scanner->yy_start;
	scanner->yy_last = scanner->yy_start;
	scanner->filecontent_ptr = NULL;
	scanner->yylineno = 1;
	scanner->yycolumn = 1;	
	++self->scanner_context_stack_num;
	return E_TLIBC_NOERROR;
ERROR_RET:
	return ret;
}

void tlibc_xml_reader_pop_file(tlibc_xml_reader_t *self)
{
	tlibc_xml_reader_pop_buff(self);
	free(self->scanner_context_stack[self->scanner_context_stack_num].filecontent_ptr);
}

void tlibc_xml_reader_pop_buff(tlibc_xml_reader_t *self)
{
	--self->scanner_context_stack_num;
}

void tlibc_xml_reader_init(tlibc_xml_reader_t *self)
{
	tlibc_abstract_reader_init(&self->super);

	self->super.read_struct_begin = tlibc_xml_read_struct_begin;
	self->super.read_struct_end = tlibc_xml_read_struct_end;
	self->super.read_union_begin = tlibc_xml_read_struct_begin;
	self->super.read_union_end = tlibc_xml_read_struct_end;
	self->super.read_enum_begin = tlibc_xml_read_enum_begin;

	self->super.read_vector_begin = tlibc_xml_read_vector_begin;
	self->super.read_vector_end = tlibc_xml_read_vector_end;
	self->super.read_field_begin = tlibc_xml_read_field_begin;
	self->super.read_field_end = tlibc_xml_read_field_end;
	self->super.read_vector_element_begin = tlibc_xml_read_vector_element_begin;
	self->super.read_vector_element_end = tlibc_xml_read_vector_element_end;

	self->super.read_int8 = tlibc_xml_read_int8;
	self->super.read_int16 = tlibc_xml_read_int16;
	self->super.read_int32 = tlibc_xml_read_int32;
	self->super.read_int64 = tlibc_xml_read_int64;

	self->super.read_uint8 = tlibc_xml_read_uint8;
	self->super.read_uint16 = tlibc_xml_read_uint16;
	self->super.read_uint32 = tlibc_xml_read_uint32;
	self->super.read_uint64 = tlibc_xml_read_uint64;

	self->super.read_double = tlibc_xml_read_double;
	self->super.read_string = tlibc_xml_read_string;
	self->super.read_char = tlibc_xml_read_char;

	self->pre_read_uint32_field_once = FALSE;
	self->ignore_int32_once = FALSE;
	self->scanner_context_stack_num = 0;
	self->include_num = 0;
	self->struct_deep = 0;
}

 tlibc_error_code_t tlibc_xml_add_include(tlibc_xml_reader_t *self, const char *path)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	if(self->include_num >= TLIBC_XML_MAX_INCLUDE)
	{
		ret = E_TLIBC_OUT_OF_MEMORY;
		goto done;
	}
	self->include[self->include_num] = path;
	++self->include_num;
done:
	return ret;
}

tlibc_error_code_t tlibc_xml_read_struct_begin(tlibc_abstract_reader_t *super, const char *struct_name)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);	
	tlibc_xml_reader_token_t token;

	if(self->struct_deep != 0)
	{
		++self->struct_deep;
		goto done;
	}

	token = tlibc_xml_reader_get_token(self);
	if(token != tok_tag_begin)
	{
		if(self->error_code == E_TLIBC_NOERROR)
		{
			ret = E_TLIBC_BAD_FILE;
		}
		else
		{
			ret = self->error_code;
		}
		goto done;
	}

	if(strcmp(struct_name, self->scanner_context_stack[self->scanner_context_stack_num - 1].tag_name) != 0)
	{
		ret = E_TLIBC_MISMATCH;
		goto done;
	}
	++self->struct_deep;
done:	
	return E_TLIBC_NOERROR;
}

tlibc_error_code_t tlibc_xml_read_struct_end(tlibc_abstract_reader_t *super, const char *struct_name)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	tlibc_xml_reader_token_t token;

	if(self->struct_deep != 0)
	{
		--self->struct_deep;
		goto done;		
	}

	token = tlibc_xml_reader_get_token(self);
	if(token != tok_tag_end)
	{
		if(self->error_code == E_TLIBC_NOERROR)
		{
			ret = E_TLIBC_BAD_FILE;
		}
		else
		{
			ret = self->error_code;
		}
		goto done;
	}
	if(strcmp(struct_name, self->scanner_context_stack[self->scanner_context_stack_num - 1].tag_name) != 0)
	{
		ret = E_TLIBC_MISMATCH;
		goto done;
	}

	--self->struct_deep;	
done:	
	return ret;
}

tlibc_error_code_t tlibc_xml_read_enum_begin(tlibc_abstract_reader_t *super, const char *enum_name)
{
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	TLIBC_UNUSED(enum_name);

	self->ignore_int32_once = TRUE;

	return E_TLIBC_NOERROR;
}

tlibc_error_code_t tlibc_xml_read_vector_begin(tlibc_abstract_reader_t *super, const char *vec_name)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	uint32_t level;
	uint32_t count;
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	//copy一份是因为统计数组的长度。
	tlibc_xml_reader_t self_copy = *self;
	TLIBC_UNUSED(vec_name);
	count = 0;
	level = 0;
	do
	{
		tlibc_xml_reader_token_t token = tlibc_xml_reader_get_token(&self_copy);
		switch(token)
		{
			case tok_tag_begin:				
				if(level == 1)
				{
					++count;
				}
				++level;
				break;
			case tok_tag_end:
				--level;
				break;
			default:				
				if(self->error_code == E_TLIBC_NOERROR)
				{
					ret = E_TLIBC_BAD_FILE;
				}
				else
				{
					ret = self->error_code;
				}
				goto ERROR_RET;
		}
	}while(level != 0);

	ret = tlibc_xml_read_field_begin(super, vec_name);
	self->pre_read_uint32_field_once = TRUE;
	self->ui32 = count;

	return ret;
ERROR_RET:
	return ret;
}

tlibc_error_code_t tlibc_xml_read_vector_end(tlibc_abstract_reader_t *super, const char *vec_name)
{
	return tlibc_xml_read_field_end(super, vec_name);
}

tlibc_error_code_t tlibc_xml_read_field_begin(tlibc_abstract_reader_t *super, const char *var_name)
{	
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	tlibc_error_code_t ret;
	tlibc_xml_reader_token_t token;	

	if(self->pre_read_uint32_field_once)
	{
		goto done;
	}
	token = tlibc_xml_reader_get_token(self);
	if(token != tok_tag_begin)
	{
		if(self->error_code == E_TLIBC_NOERROR)
		{
			ret = E_TLIBC_BAD_FILE;
		}
		else
		{
			ret = self->error_code;
		}
		goto ERROR_RET;
	}
	if(strcmp(self->scanner_context_stack[self->scanner_context_stack_num - 1].tag_name, var_name) != 0)
	{
		ret = E_TLIBC_MISMATCH;
		goto ERROR_RET;
	}

done:
	return E_TLIBC_NOERROR;
ERROR_RET:
	return ret;
}

tlibc_error_code_t tlibc_xml_read_field_end(tlibc_abstract_reader_t *super, const char *var_name)
{
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);	
	tlibc_error_code_t ret;
	tlibc_xml_reader_token_t token;

	if(self->pre_read_uint32_field_once)
	{
		self->pre_read_uint32_field_once = FALSE;
		goto done;
	}
	token = tlibc_xml_reader_get_token(self);
	if(token != tok_tag_end)
	{
		if(self->error_code == E_TLIBC_NOERROR)
		{
			ret = E_TLIBC_BAD_FILE;
		}
		else
		{
			ret = self->error_code;
		}
		goto ERROR_RET;
	}
	if(strcmp(self->scanner_context_stack[self->scanner_context_stack_num - 1].tag_name, var_name) != 0)
	{
		ret = E_TLIBC_MISMATCH;
		goto ERROR_RET;
	}

done:
	return E_TLIBC_NOERROR;
ERROR_RET:
	return ret;
}

tlibc_error_code_t tlibc_xml_read_vector_element_begin(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index)
{
	TLIBC_UNUSED(var_name);
	TLIBC_UNUSED(index);
	return tlibc_xml_read_field_begin(self, "element");
}

tlibc_error_code_t tlibc_xml_read_vector_element_end(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index)
{
	TLIBC_UNUSED(var_name);
	TLIBC_UNUSED(index);
	return tlibc_xml_read_field_end(self, "element");
}

tlibc_error_code_t tlibc_xml_read_double(tlibc_abstract_reader_t *super, double *val)
{
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	tlibc_error_code_t ret;
	
	errno = 0;
	*val = strtod(self->scanner_context_stack[self->scanner_context_stack_num - 1].content_begin, NULL);
	if(errno != 0)
	{
		ret = E_TLIBC_ERRNO;
		goto ERROR_RET;
	}

	return E_TLIBC_NOERROR;
ERROR_RET:
	return ret;
}

tlibc_error_code_t tlibc_xml_read_int8(tlibc_abstract_reader_t *super, int8_t *val)
{
	int64_t i64;
	tlibc_error_code_t ret = tlibc_xml_read_int64(super, &i64);
	*val = (int8_t)i64;
	if(*val != i64)
	{
		return E_TLIBC_INTEGER_OVERFLOW;
	}
	return ret;
}

tlibc_error_code_t tlibc_xml_read_int16(tlibc_abstract_reader_t *super, int16_t *val)
{
	int64_t i64;
	tlibc_error_code_t ret = tlibc_xml_read_int64(super, &i64);
	*val = (int16_t)i64;
	if(*val != i64)
	{
		return E_TLIBC_INTEGER_OVERFLOW;
	}
	return ret;
}

tlibc_error_code_t tlibc_xml_read_int32(tlibc_abstract_reader_t *super, int32_t *val)
{
	int64_t i64;
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	if(self->ignore_int32_once)
	{
		self->ignore_int32_once = FALSE;
		ret = E_TLIBC_PLEASE_READ_ENUM_NAME;
		goto done;
	}
	
	ret = tlibc_xml_read_int64(super, &i64);
	*val = (int32_t)i64;
	if(*val != i64)
	{
		return E_TLIBC_INTEGER_OVERFLOW;
	}
done:
	return ret;
}

tlibc_error_code_t tlibc_xml_read_int64(tlibc_abstract_reader_t *super, int64_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);

	errno = 0;
	*val = strtoll(self->scanner_context_stack[self->scanner_context_stack_num - 1].content_begin, NULL, 10);
	if(errno != 0)
	{
		ret = E_TLIBC_ERRNO;
		goto ERROR_RET;
	}

	return E_TLIBC_NOERROR;
ERROR_RET:
	return ret;
}

tlibc_error_code_t tlibc_xml_read_uint8(tlibc_abstract_reader_t *super, uint8_t *val)
{
	uint64_t ui64;
	tlibc_error_code_t ret = tlibc_xml_read_uint64(super, &ui64);
	*val = (uint8_t)ui64;
	if(*val != ui64)
	{
		return E_TLIBC_INTEGER_OVERFLOW;
	}
	return ret;
}

tlibc_error_code_t tlibc_xml_read_uint16(tlibc_abstract_reader_t *super, uint16_t *val)
{	
	uint64_t ui64;
	tlibc_error_code_t ret;

	ret = tlibc_xml_read_uint64(super, &ui64);

	*val = (uint16_t)ui64;
	if(*val != ui64)
	{
		return E_TLIBC_INTEGER_OVERFLOW;
	}
	return ret;
}

tlibc_error_code_t tlibc_xml_read_uint32(tlibc_abstract_reader_t *super, uint32_t *val)
{
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	uint64_t ui64;
	tlibc_error_code_t ret;

	if(self->pre_read_uint32_field_once)
	{
		*val = self->ui32;
		ret = E_TLIBC_NOERROR;
	}
	else
	{
		ret = tlibc_xml_read_uint64(super, &ui64);
		*val = (uint32_t)ui64;
		if(*val != ui64)
		{
			return E_TLIBC_INTEGER_OVERFLOW;
		}
	}	
	return ret;
}

tlibc_error_code_t tlibc_xml_read_uint64(tlibc_abstract_reader_t *super, uint64_t *val)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	errno = 0;
	*val = strtoull(self->scanner_context_stack[self->scanner_context_stack_num - 1].content_begin, NULL, 10);
	if(errno != 0)
	{
		ret = E_TLIBC_ERRNO;
		goto ERROR_RET;
	}

	return E_TLIBC_NOERROR;
ERROR_RET:
	return ret;
}


const char* tlibc_xml_str2c(const char* curr, const char* limit, char *ch)
{
	char c;
	if(curr >= limit)
	{
		goto ERROR_RET;
	}
	c = *curr++;
	
	if(c == '&')
	{
		char c2;
		if(curr >= limit)
		{
			goto ERROR_RET;
		}
		c2 = *curr++;
		if(c2 == 'l')
		{
			//&lt
			*ch = '<';
			curr += 2;
			if(curr >= limit)
			{
				goto ERROR_RET;
			}
		}
		else if(c2 == 'g')
		{
			//&gt
			*ch = '>';
			curr += 2;
			if(curr >= limit)
			{
				goto ERROR_RET;
			}
		}
		else
		{
			char c3;
			if(curr >= limit)
			{
				goto ERROR_RET;
			}
			c3 = *curr++;
			if(c3 == 'm')
			{
				//&amp
				*ch = '&';
				curr += 2;
				if(curr >= limit)
				{
					goto ERROR_RET;
				}
			}
			else if(c3 == 'p')
			{
				//&apos
				*ch = '\'';
				curr += 3;
				if(curr >= limit)
				{
					goto ERROR_RET;
				}
			}
			else if(c3 == 'u')
			{
				//&auot
				*ch = '\"';
				curr += 3;
				if(curr >= limit)
				{
					goto ERROR_RET;
				}
			}
		}
	}
	else
	{
		*ch = c;
	}

	return curr;
ERROR_RET:
	return NULL;
}

tlibc_error_code_t tlibc_xml_read_char(tlibc_abstract_reader_t *super, char *val)
{
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	const char* ret = tlibc_xml_str2c(self->scanner_context_stack[self->scanner_context_stack_num - 1].content_begin
		, self->scanner_context_stack[self->scanner_context_stack_num - 1].yy_limit, val);
	self->scanner_context_stack[self->scanner_context_stack_num - 1].yy_cursor = ret;
	if(ret == NULL)
	{
		return E_TLIBC_OUT_OF_MEMORY;
	}
	return E_TLIBC_NOERROR;
}

tlibc_error_code_t tlibc_xml_read_string(tlibc_abstract_reader_t *super, char *str, uint32_t str_len)
{
	tlibc_xml_reader_t *self = TLIBC_CONTAINER_OF(super, tlibc_xml_reader_t, super);
	uint32_t len = 0;
	tlibc_error_code_t ret;
	const char* curr = self->scanner_context_stack[self->scanner_context_stack_num - 1].content_begin;
	const char* limit = self->scanner_context_stack[self->scanner_context_stack_num - 1].yy_limit;
	while(curr < limit)
	{
		char c = 0;
		if(*curr == '<')
		{
			self->scanner_context_stack[self->scanner_context_stack_num - 1].yy_cursor = curr - 1;
			break;
		}
		curr = tlibc_xml_str2c(curr, limit, &c);
		if(curr == NULL)
		{
			ret = E_TLIBC_OUT_OF_MEMORY;
			goto ERROR_RET;
		}

		if(len >= str_len)
		{
			ret = E_TLIBC_OUT_OF_MEMORY;
			goto ERROR_RET;
		}
		str[len++] = c;		
	}
	str[len] = 0;
	

	return E_TLIBC_NOERROR;
ERROR_RET:
	return ret;
}

const tlibc_xml_reader_yyltype_t* tlibc_xml_current_location(tlibc_xml_reader_t *self)
{
	if(self->scanner_context_stack_num > 0)
	{
		return &self->scanner_context_stack[self->scanner_context_stack_num - 1].yylloc;
	}
	else
	{
		return NULL;
	}
}
