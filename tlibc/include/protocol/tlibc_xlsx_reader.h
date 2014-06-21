#ifndef _H_TLIBC_XLSX_READER
#define _H_TLIBC_XLSX_READER

#ifdef  __cplusplus
extern "C" {
#endif


#include "platform/tlibc_platform.h"
#include "protocol/tlibc_abstract_reader.h"
#include "core/tlibc_error_code.h"
#include "core/tlibc_unzip.h"
#include "core/tlibc_hash.h"

typedef struct tlibc_xlsx_reader_scanner_s
{
	int state;
	char *cursor;
	char *limit;
	char *marker;
}tlibc_xlsx_reader_scanner_t;

typedef struct _tlibc_xlsx_pos
{
	uint32_t row;
	uint32_t col;
}tlibc_xlsx_pos;

typedef struct _tlibc_xlsx_cell_s
{
	tlibc_hash_head_t name2index;
	int empty;
	const char* xpos;

	const char* val_start;
	const char* val_limit;

	const char* val_begin;
	const char* val_end;
}tlibc_xlsx_cell_s;

#define TLIBC_XLSX_HASH_BUCKET 65536
#define TLIBC_XLSX_MAX_COL_STR 1024

typedef struct _tlibc_xlsx_reader_t
{
	tlibc_abstract_reader_t super;

	tlibc_unzip_s unzip;

	char* workbook_rels_buff;
	uint32_t workbook_rels_buff_size;

	char* workbook_buff;
	uint32_t workbook_buff_size;

	char* sharedstring_buff;
	uint32_t sharedstring_buff_size;

	char* sheet_buff;
	uint32_t sheet_buff_size;

	tlibc_xlsx_reader_scanner_t scanner;

	char** sharedstring_begin_list;
	char** sharedstring_end_list;
	uint32_t sharedstring_list_num;

	tlibc_xlsx_pos cell_min_pos;
	tlibc_xlsx_pos cell_max_pos;
	uint32_t cell_row_size;
	uint32_t cell_col_size;
	tlibc_xlsx_cell_s *cell_matrix;
	tlibc_xlsx_cell_s *bindinfo_row;

	tlibc_xlsx_cell_s *curr_row;
	tlibc_xlsx_cell_s *curr_cell;	

	


	int32_t last_col;
	char last_col_str[TLIBC_XLSX_MAX_COL_STR];

	int read_enum_name_once;
	tlibc_hash_bucket_t hash_bucket[TLIBC_XLSX_HASH_BUCKET];
	tlibc_hash_t name2index;

	int pre_read_uint32;
	uint32_t pre_u32;


	//如果不存在动态长度的数据， 可以通过这个开关来优化读取速度。
	//可以通过TData生成数据的标号来更好的解决这个问题。
	int use_cache;
	size_t row_index;
	tlibc_xlsx_cell_s **hash_cache;
}tlibc_xlsx_reader_t;

tlibc_error_code_t tlibc_xlsx_reader_init(tlibc_xlsx_reader_t *self, const char *file_name);

tlibc_error_code_t tlibc_xlsx_reader_open_sheet(tlibc_xlsx_reader_t *self, const char* sheet, uint32_t bindinfo_row);

uint32_t tlibc_xlsx_reader_num_rows(tlibc_xlsx_reader_t *self);

void tlibc_xlsx_reader_row_seek(tlibc_xlsx_reader_t *self, uint32_t offset);

void tlibc_xlsx_reader_close_sheet(tlibc_xlsx_reader_t *self);

void tlibc_xlsx_reader_fini(tlibc_xlsx_reader_t *self);

size_t tlibc_xlsx_str2num(const char* str);

const char* tlibc_xlsx_num2str(int num, char *str, size_t str_max_len);

size_t tlibc_xlsx_current_col(tlibc_xlsx_reader_t *self);


tlibc_error_code_t tlibc_xlsx_read_vector_begin(tlibc_abstract_reader_t *super, const char* vec_name);

tlibc_error_code_t tlibc_xlsx_read_vector_element_begin(tlibc_abstract_reader_t *super, const char* var_name, uint32_t index);

tlibc_error_code_t tlibc_xlsx_read_field_begin(tlibc_abstract_reader_t *super, const char *var_name);

tlibc_error_code_t tlibc_xlsx_read_enum_begin(tlibc_abstract_reader_t *super, const char *enum_name);

tlibc_error_code_t tlibc_xlsx_read_char(tlibc_abstract_reader_t *super, char *val);

tlibc_error_code_t tlibc_xlsx_read_double(tlibc_abstract_reader_t *super, double *val);

tlibc_error_code_t tlibc_xlsx_read_int8(tlibc_abstract_reader_t *super, int8_t *val);

tlibc_error_code_t tlibc_xlsx_read_int16(tlibc_abstract_reader_t *super, int16_t *val);

tlibc_error_code_t tlibc_xlsx_read_int32(tlibc_abstract_reader_t *super, int32_t *val);

tlibc_error_code_t tlibc_xlsx_read_int64(tlibc_abstract_reader_t *super, int64_t *val);

tlibc_error_code_t tlibc_xlsx_read_uint8(tlibc_abstract_reader_t *super, uint8_t *val);

tlibc_error_code_t tlibc_xlsx_read_uint16(tlibc_abstract_reader_t *super, uint16_t *val);

tlibc_error_code_t tlibc_xlsx_read_uint32(tlibc_abstract_reader_t *super, uint32_t *val);

tlibc_error_code_t tlibc_xlsx_read_uint64(tlibc_abstract_reader_t *super, uint64_t *val);

tlibc_error_code_t tlibc_xlsx_read_string(tlibc_abstract_reader_t *super, char *str, uint32_t str_len);

#ifdef  __cplusplus
}
#endif

#endif
