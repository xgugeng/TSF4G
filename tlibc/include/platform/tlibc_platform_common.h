#ifndef _H_TLIBC_PLATFORM_COMMON
#define _H_TLIBC_PLATFORM_COMMON

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdint.h>

#ifndef NULL
#define NULL				0
#endif

#ifndef TRUE
#define TRUE				1
#endif

#ifndef FALSE
#define FALSE				0
#endif


#ifndef TLIBC_IN
#define TLIBC_IN
#endif

#ifndef TLIBC_OUT
#define TLIBC_OUT
#endif

#ifndef TLIBC_INOUT
#define TLIBC_INOUT
#endif

#define TLIBC_UNUSED(x) (void)(x)

#define TLIBC_FILE_SEPARATOR '/'


#define TLIBC_UINT64_MAX ((((uint64_t)0xffffffffUL) << 32) | 0xffffffffUL)
#define TLIBC_UINT32_MAX ((uint32_t)0xffffffffUL)
#define TLIBC_UINT16_MAX ((uint16_t)0xffffUL)
#define TLIBC_UINT8_MAX  255

#define TLIBC_INT64_MAX  ((((int64_t) 0x7fffffffL) << 32) | 0xffffffffL)
#define TLIBC_INT64_MIN  ((-TLIBC_INT64_MAX) - 1)
#define TLIBC_INT32_MAX  ((int32_t) 0x7fffffffL)
#define TLIBC_INT32_MIN  ((-TLIBC_INT32_MAX) - 1)
#define TLIBC_INT16_MAX  ((int16_t) 0x7fffL)
#define TLIBC_INT16_MIN  ((-TLIBC_INT16_MAX) - 1)
#define TLIBC_INT8_MAX   127
#define TLIBC_INT8_MIN   ((-TLIBC_INT8_MAX) - 1)

#define TLIBC_BYTE_ORDER (0x1234 & 0xff)
#define TLIBC_LITTLE_ENDIAN (0x34)
#define TLIBC_BIG_ENDIAN (0x12)

#define TLIBC_MAX_LENGTH_OF_IDENTIFIER 255


#define TLIBC_OFFSET_OF(type, member) ((size_t) &((type *)0)->member)

#define TLIBC_CONTAINER_OF(ptr, type, member) ((type *)((size_t)ptr - TLIBC_OFFSET_OF(type, member)))

#define TLIBC_MAX_PATH_LENGTH 1024


#ifdef  __cplusplus
}
#endif

#endif
