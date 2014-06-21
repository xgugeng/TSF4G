#ifndef _H_TLIBC_XML_READER
#define _H_TLIBC_XML_READER

#ifdef  __cplusplus
extern "C" {
#endif


#include "platform/tlibc_platform.h"
#include "protocol/tlibc_abstract_reader.h"
#include "core/tlibc_error_code.h"

typedef struct tlibc_xml_reader_yyltype_s
{
	char file_name[TLIBC_MAX_PATH_LENGTH];
	int first_line;
	int first_column;
	int last_line;
	int last_column;
} tlibc_xml_reader_yyltype_t;

typedef struct tlibc_xml_reader_scanner_context_s tlibc_xml_reader_scanner_context_t;
struct tlibc_xml_reader_scanner_context_s
{
	int yy_state;
	const char *yy_last;
	const char *yy_cursor;
	const char *yy_limit;	
	const char *yy_marker;
	const char *yy_start;

	const char *yy_text;
	uint32_t yy_leng;

	uint32_t yylineno;
	uint32_t yycolumn;

	tlibc_xml_reader_yyltype_t yylloc;

	char tag_name[TLIBC_MAX_LENGTH_OF_IDENTIFIER];
	const char *content_begin;

	void *filecontent_ptr;
};

#define TLIBC_XML_MAX_INCLUDE 1024
#define TLIBC_XML_MAX_DEEP 1024
typedef struct tlibc_xml_reader_s
{
	tlibc_abstract_reader_t super;

	tlibc_xml_reader_scanner_context_t scanner_context_stack[TLIBC_XML_MAX_DEEP];
	size_t scanner_context_stack_num;

	const char *include[TLIBC_XML_MAX_INCLUDE];
	size_t include_num;

	int struct_deep;
	int ignore_int32_once;
	int pre_read_uint32_field_once;
	uint32_t ui32;
	tlibc_error_code_t error_code;
}tlibc_xml_reader_t;

void tlibc_xml_reader_init(tlibc_xml_reader_t *self);

tlibc_error_code_t tlibc_xml_add_include(tlibc_xml_reader_t *self, const char *path);

tlibc_error_code_t tlibc_xml_reader_push_file(tlibc_xml_reader_t *self, const char *file_name);

tlibc_error_code_t tlibc_xml_reader_push_buff(tlibc_xml_reader_t *self, const char *xml_start, const char* xml_limit);

void tlibc_xml_reader_pop_file(tlibc_xml_reader_t *self);

void tlibc_xml_reader_pop_buff(tlibc_xml_reader_t *self);

const char* tlibc_xml_str2c(const char* curr, const char* limit, char *ch);

const tlibc_xml_reader_yyltype_t* tlibc_xml_current_location(tlibc_xml_reader_t *self);



tlibc_error_code_t tlibc_xml_read_struct_begin(tlibc_abstract_reader_t *self, const char *struct_name);

tlibc_error_code_t tlibc_xml_read_struct_end(tlibc_abstract_reader_t *self, const char *struct_name);

tlibc_error_code_t tlibc_xml_read_enum_begin(tlibc_abstract_reader_t *self, const char *enum_name);

tlibc_error_code_t tlibc_xml_read_vector_begin(tlibc_abstract_reader_t *self, const char *vec_name);

tlibc_error_code_t tlibc_xml_read_vector_end(tlibc_abstract_reader_t *self, const char *vec_name);

tlibc_error_code_t tlibc_xml_read_field_begin(tlibc_abstract_reader_t *self, const char *var_name);

tlibc_error_code_t tlibc_xml_read_field_end(tlibc_abstract_reader_t *self, const char *var_name);

tlibc_error_code_t tlibc_xml_read_vector_element_begin(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index);

tlibc_error_code_t tlibc_xml_read_vector_element_end(tlibc_abstract_reader_t *self, const char *var_name, uint32_t index);

tlibc_error_code_t tlibc_xml_read_char(tlibc_abstract_reader_t *super, char *val);

tlibc_error_code_t tlibc_xml_read_double(tlibc_abstract_reader_t *super, double *val);

tlibc_error_code_t tlibc_xml_read_int8(tlibc_abstract_reader_t *super, int8_t *val);

tlibc_error_code_t tlibc_xml_read_int16(tlibc_abstract_reader_t *super, int16_t *val);

tlibc_error_code_t tlibc_xml_read_int32(tlibc_abstract_reader_t *super, int32_t *val);

tlibc_error_code_t tlibc_xml_read_int64(tlibc_abstract_reader_t *super, int64_t *val);

tlibc_error_code_t tlibc_xml_read_uint8(tlibc_abstract_reader_t *super, uint8_t *val);

tlibc_error_code_t tlibc_xml_read_uint16(tlibc_abstract_reader_t *super, uint16_t *val);

tlibc_error_code_t tlibc_xml_read_uint32(tlibc_abstract_reader_t *super, uint32_t *val);

tlibc_error_code_t tlibc_xml_read_uint64(tlibc_abstract_reader_t *super, uint64_t *val);

tlibc_error_code_t tlibc_xml_read_string(tlibc_abstract_reader_t *super, char *str, uint32_t str_len);

#ifdef  __cplusplus
}
#endif


#endif

