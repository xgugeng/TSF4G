#ifndef _H_TLIBC_CSV_READER_H
#define _H_TLIBC_CSV_READER_H

#include "protocol/tlibc_abstract_reader.h"
#include "tlibc_error_code.h"
#include <stdint.h>
#include <stddef.h>

#define TLIBC_CSV_FIELD_NUM 65535

typedef struct tlibc_csv_reader_s
{
	tlibc_abstract_reader_t super;
	char *top_line;
	size_t top_line_size;
	char* top_line_fields[TLIBC_CSV_FIELD_NUM];
	size_t	top_line_fields_num;

	char* cur_line_fields[TLIBC_CSV_FIELD_NUM];
	size_t	cur_line_fields_num;
	char *cur_line;
	size_t cur_line_size;

	bool pre_read_uint32;
	int32_t col;
	int32_t field_index[TLIBC_CSV_FIELD_NUM];
	char* field;
	char* field_end;
	bool read_enum_name_once;
	uint32_t pre_u32;
}tlibc_csv_reader_t;


tlibc_error_code_t tlibc_csv_reader_init(tlibc_csv_reader_t *self, const char *top_line, size_t top_line_size);

void tlibc_csv_reader_fini(tlibc_csv_reader_t *self);

tlibc_error_code_t tlibc_csv_reader_store(tlibc_csv_reader_t *self, const char *line, size_t line_size);

void tlibc_csv_reader_close(tlibc_csv_reader_t *self);



tlibc_error_code_t tlibc_csv_read_vector_begin(tlibc_abstract_reader_t *super, const char* vec_name);

tlibc_error_code_t tlibc_csv_read_vector_element_begin(tlibc_abstract_reader_t *super, const char* var_name, uint32_t index);

tlibc_error_code_t tlibc_csv_read_vector_element_end(tlibc_abstract_reader_t *super, const char* var_name, uint32_t index);

tlibc_error_code_t tlibc_csv_read_field_begin(tlibc_abstract_reader_t *super, const char *var_name);

tlibc_error_code_t tlibc_csv_read_enum_begin(tlibc_abstract_reader_t *super, const char *enum_name);

tlibc_error_code_t tlibc_csv_read_int8(tlibc_abstract_reader_t *super, int8_t *val);

tlibc_error_code_t tlibc_csv_read_int16(tlibc_abstract_reader_t *super, int16_t *val);

tlibc_error_code_t tlibc_csv_read_int32(tlibc_abstract_reader_t *super, int32_t *val);

tlibc_error_code_t tlibc_csv_read_int64(tlibc_abstract_reader_t *super, int64_t *val);

tlibc_error_code_t tlibc_csv_read_uint8(tlibc_abstract_reader_t *super, uint8_t *val);

tlibc_error_code_t tlibc_csv_read_uint16(tlibc_abstract_reader_t *super, uint16_t *val);

tlibc_error_code_t tlibc_csv_read_uint32(tlibc_abstract_reader_t *super, uint32_t *val);

tlibc_error_code_t tlibc_csv_read_uint64(tlibc_abstract_reader_t *super, uint64_t *val);

tlibc_error_code_t tlibc_csv_read_bool(tlibc_abstract_reader_t *super, bool *val);

tlibc_error_code_t tlibc_csv_read_double(tlibc_abstract_reader_t *super, double *val);

tlibc_error_code_t tlibc_csv_read_char(tlibc_abstract_reader_t *super, char *val);

tlibc_error_code_t tlibc_csv_read_string(tlibc_abstract_reader_t *super, char *str, uint32_t str_len);

#endif //_H_TLIBC_CSV_READER_H

