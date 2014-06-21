#ifndef _H_TLIBC_PLATFORM_LINUX
#define _H_TLIBC_PLATFORM_LINUX

#ifdef  __cplusplus
extern "C" {
#endif


#define TLIBC_FILE_SEPARATOR '/'

#include <stddef.h>
#include <inttypes.h>
#include <sys/stat.h>
//如果不包含stdlib.h strtoll会出错
#include <stdlib.h>

#if __WORDSIZE == 32
#define TLIBC_WORDSIZE 32
#elif __WORDSIZE == 64
#define TLIBC_WORDSIZE 64
#else
#error "unknow wordsize"
#endif

#ifdef  __cplusplus
}
#endif


#endif
