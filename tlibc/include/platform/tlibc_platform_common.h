#ifndef _H_TLIBC_PLATFORM_COMMON
#define _H_TLIBC_PLATFORM_COMMON

#include <stdint.h>


#define TLIBC_UNUSED(x) (void)(x)





#define TLIBC_MAX_LENGTH_OF_IDENTIFIER 255


#define TLIBC_OFFSET_OF(type, member) ((size_t) &((type *)0)->member)

#define TLIBC_CONTAINER_OF(ptr, type, member) ((type *)((size_t)ptr - TLIBC_OFFSET_OF(type, member)))

#define TLIBC_MAX_PATH_LENGTH 1024


#ifdef  __cplusplus
}
#endif

#endif
