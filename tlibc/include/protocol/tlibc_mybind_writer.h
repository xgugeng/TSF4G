#ifndef _H_TLIBC_BIND_WRITER
#define _H_TLIBC_BIND_WRITER

#ifdef  __cplusplus
extern "C" {
#endif


#include "platform/tlibc_platform.h"
#include "protocol/tlibc_abstract_writer.h"
#include "core/tlibc_error_code.h"

#include <stdio.h>
#include <stdint.h>


#include "mysql.h"

typedef struct tlibc_mybind_writer_s
{
	tlibc_abstract_writer_t super;
	MYSQL_BIND *bind_vec;
	uint32_t bind_vec_num;
	uint32_t idx;
}tlibc_mybind_writer_t;

void tlibc_mybind_writer_init(tlibc_mybind_writer_t *self, MYSQL_BIND *bind_vec, uint32_t bind_vec_num);

tlibc_error_code_t tlibc_mybind_write_int8(tlibc_abstract_writer_t *super, const int8_t *val);

tlibc_error_code_t tlibc_mybind_write_int16(tlibc_abstract_writer_t *super, const int16_t *val);

tlibc_error_code_t tlibc_mybind_write_int32(tlibc_abstract_writer_t *super, const int32_t *val);

tlibc_error_code_t tlibc_mybind_write_int64(tlibc_abstract_writer_t *super, const int64_t *val);

tlibc_error_code_t tlibc_mybind_write_uint8(tlibc_abstract_writer_t *super, const uint8_t *val);

tlibc_error_code_t tlibc_mybind_write_uint16(tlibc_abstract_writer_t *super, const uint16_t *val);

tlibc_error_code_t tlibc_mybind_write_uint32(tlibc_abstract_writer_t *super, const uint32_t *val);

tlibc_error_code_t tlibc_mybind_write_uint64(tlibc_abstract_writer_t *super, const uint64_t *val);

tlibc_error_code_t tlibc_mybind_write_double(tlibc_abstract_writer_t *super, const double *val);

tlibc_error_code_t tlibc_mybind_write_char(tlibc_abstract_writer_t *super, const char *val);

tlibc_error_code_t tlibc_mybind_write_string(tlibc_abstract_writer_t *super, const char* str, uint32_t str_length);


#ifdef  __cplusplus
}
#endif

#endif
