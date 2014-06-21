#ifndef _H_TLIBC_ABSTRACT_WRITER
#define _H_TLIBC_ABSTRACT_WRITER

#ifdef  __cplusplus
extern "C" {
#endif

#include "platform/tlibc_platform.h"
#include "core/tlibc_error_code.h"
#include <stdint.h>

typedef struct tlibc_abstract_writer_s tlibc_abstract_writer_t;
struct tlibc_abstract_writer_s
{
	tlibc_error_code_t (*write_struct_begin)(tlibc_abstract_writer_t *self, const char *struct_name);
	tlibc_error_code_t (*write_struct_end)(tlibc_abstract_writer_t *self, const char *struct_name);
	tlibc_error_code_t (*write_union_begin)(tlibc_abstract_writer_t *self, const char *union_name);
	tlibc_error_code_t (*write_union_end)(tlibc_abstract_writer_t *self, const char *union_name);
	tlibc_error_code_t (*write_enum_begin)(tlibc_abstract_writer_t *self, const char *enum_name);
	tlibc_error_code_t (*write_enum_end)(tlibc_abstract_writer_t *self, const char *enum_name);
	tlibc_error_code_t (*write_vector_begin)(tlibc_abstract_writer_t *self, const char* vec_name);
	tlibc_error_code_t (*write_vector_end)(tlibc_abstract_writer_t *self, const char* vec_name);
	tlibc_error_code_t (*write_field_begin)(tlibc_abstract_writer_t *self, const char *var_name);
	tlibc_error_code_t (*write_field_end)(tlibc_abstract_writer_t *self, const char *var_name);	
	tlibc_error_code_t (*write_vector_element_begin)(tlibc_abstract_writer_t *self, const char *var_name, uint32_t index);
	tlibc_error_code_t (*write_vector_element_end)(tlibc_abstract_writer_t *self, const char *var_name, uint32_t index);

	tlibc_error_code_t (*write_int8)(tlibc_abstract_writer_t *self, const int8_t *val);
	tlibc_error_code_t (*write_int16)(tlibc_abstract_writer_t *self, const int16_t *val);
	tlibc_error_code_t (*write_int32)(tlibc_abstract_writer_t *self, const int32_t *val);
	tlibc_error_code_t (*write_int64)(tlibc_abstract_writer_t *self, const int64_t *val);
	tlibc_error_code_t (*write_uint8)(tlibc_abstract_writer_t *self, const uint8_t *val);
	tlibc_error_code_t (*write_uint16)(tlibc_abstract_writer_t *self, const uint16_t *val);
	tlibc_error_code_t (*write_uint32)(tlibc_abstract_writer_t *self, const uint32_t *val);
	tlibc_error_code_t (*write_uint64)(tlibc_abstract_writer_t *self, const uint64_t *val);
	tlibc_error_code_t (*write_char)(tlibc_abstract_writer_t *self, const char *val);
	tlibc_error_code_t (*write_double)(tlibc_abstract_writer_t *self, const double *val);
	tlibc_error_code_t (*write_string)(tlibc_abstract_writer_t *self, const char* str, uint32_t str_length);
};

void tlibc_abstract_writer_init(tlibc_abstract_writer_t *self);

tlibc_error_code_t tlibc_write_struct_begin(tlibc_abstract_writer_t *self, const char *struct_name);
tlibc_error_code_t tlibc_write_struct_end(tlibc_abstract_writer_t *self, const char *struct_name);
tlibc_error_code_t tlibc_write_union_begin(tlibc_abstract_writer_t *self, const char *union_name);
tlibc_error_code_t tlibc_write_union_end(tlibc_abstract_writer_t *self, const char *union_name);
tlibc_error_code_t tlibc_write_enum_begin(tlibc_abstract_writer_t *self, const char *enum_name);
tlibc_error_code_t tlibc_write_enum_end(tlibc_abstract_writer_t *self, const char *enum_name);
tlibc_error_code_t tlibc_write_vector_begin(tlibc_abstract_writer_t *self, const char* vec_name);
tlibc_error_code_t tlibc_write_vector_end(tlibc_abstract_writer_t *self, const char* vec_name);
tlibc_error_code_t tlibc_write_field_begin(tlibc_abstract_writer_t *self, const char *var_name);
tlibc_error_code_t tlibc_write_field_end(tlibc_abstract_writer_t *self, const char *var_name);
tlibc_error_code_t tlibc_write_vector_element_begin(tlibc_abstract_writer_t *self, const char *var_name, uint32_t index);
tlibc_error_code_t tlibc_write_vector_element_end(tlibc_abstract_writer_t *self, const char *var_name, uint32_t index);

tlibc_error_code_t tlibc_write_int8(tlibc_abstract_writer_t *self, const int8_t *val);
tlibc_error_code_t tlibc_write_int16(tlibc_abstract_writer_t *self, const int16_t *val);
tlibc_error_code_t tlibc_write_int32(tlibc_abstract_writer_t *self, const int32_t *val);
tlibc_error_code_t tlibc_write_int64(tlibc_abstract_writer_t *self, const int64_t *val);
tlibc_error_code_t tlibc_write_uint8(tlibc_abstract_writer_t *self, const uint8_t *val);
tlibc_error_code_t tlibc_write_uint16(tlibc_abstract_writer_t *self, const uint16_t *val);
tlibc_error_code_t tlibc_write_uint32(tlibc_abstract_writer_t *self, const uint32_t *val);
tlibc_error_code_t tlibc_write_uint64(tlibc_abstract_writer_t *self, const uint64_t *val);
tlibc_error_code_t tlibc_write_char(tlibc_abstract_writer_t *self, const char *val);	
tlibc_error_code_t tlibc_write_double(tlibc_abstract_writer_t *self, const double *val);
tlibc_error_code_t tlibc_write_string(tlibc_abstract_writer_t *self, const char *str, uint32_t str_length);

#ifdef  __cplusplus
}
#endif

#endif

