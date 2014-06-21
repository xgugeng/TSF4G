#ifndef _H_TLIBC_XML_WRITER
#define _H_TLIBC_XML_WRITER

#ifdef  __cplusplus
extern "C" {
#endif


#include "platform/tlibc_platform.h"
#include "protocol/tlibc_abstract_writer.h"
#include "core/tlibc_error_code.h"

#include <stdio.h>


typedef struct tlibc_xml_writer_s
{
	tlibc_abstract_writer_t super;
	uint32_t count;
	char *start;
	char *cur;
	char *limit;

	int skip_uint32_field_once;

	int ignore_int32_once;
}tlibc_xml_writer_t;

void tlibc_xml_writer_init(tlibc_xml_writer_t *self, char *start, char*limit);


tlibc_error_code_t tlibc_xml_write_vector_begin(tlibc_abstract_writer_t *super, const char* vec_name);

tlibc_error_code_t tlibc_xml_write_vector_end(tlibc_abstract_writer_t *super, const char* vec_name);

tlibc_error_code_t tlibc_xml_write_field_begin(tlibc_abstract_writer_t *super, const char *var_name);

tlibc_error_code_t tlibc_xml_write_field_end(tlibc_abstract_writer_t *super, const char *var_name);

tlibc_error_code_t tlibc_xml_write_vector_element_begin(tlibc_abstract_writer_t *super, const char *var_name, uint32_t index);

tlibc_error_code_t tlibc_xml_write_vector_element_end(tlibc_abstract_writer_t *super, const char *var_name, uint32_t index);

tlibc_error_code_t tlibc_xml_write_int8(tlibc_abstract_writer_t *super, const int8_t *val);

tlibc_error_code_t tlibc_xml_write_int16(tlibc_abstract_writer_t *super, const int16_t *val);

tlibc_error_code_t tlibc_xml_write_int32(tlibc_abstract_writer_t *super, const int32_t *val);

tlibc_error_code_t tlibc_xml_write_int64(tlibc_abstract_writer_t *super, const int64_t *val);

tlibc_error_code_t tlibc_xml_write_uint8(tlibc_abstract_writer_t *super, const uint8_t *val);

tlibc_error_code_t tlibc_xml_write_uint16(tlibc_abstract_writer_t *super, const uint16_t *val);

tlibc_error_code_t tlibc_xml_write_uint32(tlibc_abstract_writer_t *super, const uint32_t *val);

tlibc_error_code_t tlibc_xml_write_uint64(tlibc_abstract_writer_t *super, const uint64_t *val);

tlibc_error_code_t tlibc_xml_write_string(tlibc_abstract_writer_t *super, const char* str, uint32_t str_length);

tlibc_error_code_t tlibc_xml_write_double(tlibc_abstract_writer_t *super, const double *val);

tlibc_error_code_t tlibc_xml_write_char(tlibc_abstract_writer_t *super, const char *val);

tlibc_error_code_t tlibc_xml_write_struct_begin(tlibc_abstract_writer_t *super, const char *struct_name);

tlibc_error_code_t tlibc_xml_write_struct_end(tlibc_abstract_writer_t *super, const char *struct_name);

tlibc_error_code_t tlibc_xml_write_enum_begin(tlibc_abstract_writer_t *super, const char *enum_name);


#ifdef  __cplusplus
}
#endif

#endif
