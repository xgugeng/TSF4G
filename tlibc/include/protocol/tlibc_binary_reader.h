#ifndef _H_TLIBC_BINARY_READER
#define _H_TLIBC_BINARY_READER

#ifdef  __cplusplus
extern "C" {
#endif


#include "platform/tlibc_platform.h"
#include "protocol/tlibc_abstract_reader.h"

typedef struct tlibc_binary_reader_s
{
	tlibc_abstract_reader_t super;

	const char *addr;
	uint32_t size;
	uint32_t offset;
}tlibc_binary_reader_t;

void tlibc_binary_reader_init(tlibc_binary_reader_t *self, const char *addr, uint32_t size);

tlibc_error_code_t tlibc_binary_read_int8(tlibc_abstract_reader_t *super, int8_t *val);

tlibc_error_code_t tlibc_binary_read_int16(tlibc_abstract_reader_t *super, int16_t *val);

tlibc_error_code_t tlibc_binary_read_int32(tlibc_abstract_reader_t *super, int32_t *val);

tlibc_error_code_t tlibc_binary_read_int64(tlibc_abstract_reader_t *super, int64_t *val);

tlibc_error_code_t tlibc_binary_read_uint8(tlibc_abstract_reader_t *super, uint8_t *val);

tlibc_error_code_t tlibc_binary_read_uint16(tlibc_abstract_reader_t *super, uint16_t *val);

tlibc_error_code_t tlibc_binary_read_uint32(tlibc_abstract_reader_t *super, uint32_t *val);

tlibc_error_code_t tlibc_binary_read_uint64(tlibc_abstract_reader_t *super, uint64_t *val);

tlibc_error_code_t tlibc_binary_read_char(tlibc_abstract_reader_t *super, char *val);

tlibc_error_code_t tlibc_binary_read_double(tlibc_abstract_reader_t *super, double *val);

tlibc_error_code_t tlibc_binary_read_string(tlibc_abstract_reader_t *super, char* str, uint32_t str_length);

#ifdef  __cplusplus
}
#endif


#endif
