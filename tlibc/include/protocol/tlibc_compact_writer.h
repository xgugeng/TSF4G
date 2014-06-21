#ifndef _H_TLIBC_COMPACT_WRITER
#define _H_TLIBC_COMPACT_WRITER

#ifdef  __cplusplus
extern "C" {
#endif


#include "platform/tlibc_platform.h"
#include "protocol/tlibc_abstract_writer.h"

typedef struct tlibc_compact_writer_s
{
	tlibc_abstract_writer_t super;

	char *addr;
	uint32_t size;
	uint32_t offset;
}tlibc_compact_writer_t;

void tlibc_compact_writer_init(tlibc_compact_writer_t *self, char *addr, uint32_t size);

tlibc_error_code_t tlibc_compact_write_int8(tlibc_abstract_writer_t *super, const int8_t *val);

tlibc_error_code_t tlibc_compact_write_int16(tlibc_abstract_writer_t *super, const int16_t *val);

tlibc_error_code_t tlibc_compact_write_int32(tlibc_abstract_writer_t *super, const int32_t *val);

tlibc_error_code_t tlibc_compact_write_int64(tlibc_abstract_writer_t *super, const int64_t *val);

tlibc_error_code_t tlibc_compact_write_uint8(tlibc_abstract_writer_t *super, const uint8_t *val);

tlibc_error_code_t tlibc_compact_write_uint16(tlibc_abstract_writer_t *super, const uint16_t *val);

tlibc_error_code_t tlibc_compact_write_uint32(tlibc_abstract_writer_t *super, const uint32_t *val);

tlibc_error_code_t tlibc_compact_write_uint64(tlibc_abstract_writer_t *super, const uint64_t *val);

tlibc_error_code_t tlibc_compact_write_char(tlibc_abstract_writer_t *super, const char *val);

tlibc_error_code_t tlibc_compact_write_double(tlibc_abstract_writer_t *super, const double *val);

tlibc_error_code_t tlibc_compact_write_string(tlibc_abstract_writer_t *super, const char* str, uint32_t str_length);

#ifdef  __cplusplus
}
#endif

#endif
