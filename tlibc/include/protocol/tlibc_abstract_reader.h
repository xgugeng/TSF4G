#ifndef _H_TLIBC_ABSTRACT_READR
#define _H_TLIBC_ABSTRACT_READR

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "core/tlibc_error_code.h"

#define TLIBC_READER_NAME_LENGTH 65536

typedef struct tlibc_abstract_reader_s tlibc_abstract_reader_t;
struct tlibc_abstract_reader_s
{
	int enable_name;
	char name[TLIBC_READER_NAME_LENGTH];
	char *name_ptr;

	tlibc_error_code_t (*read_struct_begin)(tlibc_abstract_reader_t *self, const char *struct_name);
	tlibc_error_code_t (*read_struct_end)(tlibc_abstract_reader_t *self, const char *struct_name);
	tlibc_error_code_t (*read_union_begin)(tlibc_abstract_reader_t *self, const char *union_name);
	tlibc_error_code_t (*read_union_end)(tlibc_abstract_reader_t *self, const char *union_name);
	tlibc_error_code_t (*read_enum_begin)(tlibc_abstract_reader_t *self, const char *enum_name);
	tlibc_error_code_t (*read_enum_end)(tlibc_abstract_reader_t *self, const char *enum_name);
	tlibc_error_code_t (*read_vector_begin)(tlibc_abstract_reader_t *self, const char* vec_name);
	tlibc_error_code_t (*read_vector_end)(tlibc_abstract_reader_t *self, const char* vec_name);
	tlibc_error_code_t (*read_vector_element_begin)(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index);
	tlibc_error_code_t (*read_vector_element_end)(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index);
	tlibc_error_code_t (*read_field_begin)(tlibc_abstract_reader_t *self, const char *var_name);
	tlibc_error_code_t (*read_field_end)(tlibc_abstract_reader_t *self, const char *var_name);

	tlibc_error_code_t (*read_int8)(tlibc_abstract_reader_t *self, int8_t *val);
	tlibc_error_code_t (*read_int16)(tlibc_abstract_reader_t *self, int16_t *val);
	tlibc_error_code_t (*read_int32)(tlibc_abstract_reader_t *self, int32_t *val);
	tlibc_error_code_t (*read_int64)(tlibc_abstract_reader_t *self, int64_t *val);
	tlibc_error_code_t (*read_uint8)(tlibc_abstract_reader_t *self, uint8_t *val);
	tlibc_error_code_t (*read_uint16)(tlibc_abstract_reader_t *self, uint16_t *val);
	tlibc_error_code_t (*read_uint32)(tlibc_abstract_reader_t *self, uint32_t *val);
	tlibc_error_code_t (*read_uint64)(tlibc_abstract_reader_t *self, uint64_t *val);
	tlibc_error_code_t (*read_char)(tlibc_abstract_reader_t *self, char *val);
	tlibc_error_code_t (*read_double)(tlibc_abstract_reader_t *self, double *val);
	tlibc_error_code_t (*read_string)(tlibc_abstract_reader_t *self, char* str, uint32_t str_length);
};

void tlibc_abstract_reader_init(tlibc_abstract_reader_t *self);

tlibc_error_code_t tlibc_read_struct_begin(tlibc_abstract_reader_t *self, const char *struct_name);
tlibc_error_code_t tlibc_read_struct_end(tlibc_abstract_reader_t *self, const char *struct_name);
tlibc_error_code_t tlibc_read_union_begin(tlibc_abstract_reader_t *self, const char *union_name);
tlibc_error_code_t tlibc_read_union_end(tlibc_abstract_reader_t *self, const char *union_name);
tlibc_error_code_t tlibc_read_enum_begin(tlibc_abstract_reader_t *self, const char *enum_name);
tlibc_error_code_t tlibc_read_enum_end(tlibc_abstract_reader_t *self, const char *enum_name);
tlibc_error_code_t tlibc_read_vector_begin(tlibc_abstract_reader_t *self, const char* vec_name);
tlibc_error_code_t tlibc_read_vector_end(tlibc_abstract_reader_t *self, const char* vec_name);
tlibc_error_code_t tlibc_read_field_begin(tlibc_abstract_reader_t *self, const char *var_name);
tlibc_error_code_t tlibc_read_field_end(tlibc_abstract_reader_t *self, const char *var_name);
tlibc_error_code_t tlibc_read_vector_element_begin(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index);
tlibc_error_code_t tlibc_read_vector_element_end(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index);

tlibc_error_code_t tlibc_read_int8(tlibc_abstract_reader_t *self, int8_t *val);
tlibc_error_code_t tlibc_read_int16(tlibc_abstract_reader_t *self, int16_t *val);
tlibc_error_code_t tlibc_read_int32(tlibc_abstract_reader_t *self, int32_t *val);
tlibc_error_code_t tlibc_read_int64(tlibc_abstract_reader_t *self, int64_t *val);
tlibc_error_code_t tlibc_read_uint8(tlibc_abstract_reader_t *self, uint8_t *val);
tlibc_error_code_t tlibc_read_uint16(tlibc_abstract_reader_t *self, uint16_t *val);
tlibc_error_code_t tlibc_read_uint32(tlibc_abstract_reader_t *self, uint32_t *val);
tlibc_error_code_t tlibc_read_uint64(tlibc_abstract_reader_t *self, uint64_t *val);
tlibc_error_code_t tlibc_read_char(tlibc_abstract_reader_t *self, char *val);
tlibc_error_code_t tlibc_read_double(tlibc_abstract_reader_t *self, double *val);
tlibc_error_code_t tlibc_read_string(tlibc_abstract_reader_t *self, char* str, uint32_t str_length);

#ifdef  __cplusplus
}
#endif

#endif
