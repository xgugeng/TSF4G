#ifndef _H_TLIBC_ERROR_CODE
#define _H_TLIBC_ERROR_CODE

#ifdef  __cplusplus
extern "C" {
#endif

#include "platform/tlibc_platform.h"

typedef enum tlibc_error_code_e
{
    E_TLIBC_NOERROR = 0,
    E_TLIBC_ERROR = -1,	
    E_TLIBC_OUT_OF_MEMORY = -2,
	E_TLIBC_NOT_FOUND = -3,
	E_TLIBC_SYNTAX = -4,
	E_TLIBC_MISMATCH = -5,
	E_TLIBC_ERRNO = -6,
	E_TLIBC_WOULD_BLOCK = -7,
	E_TLIBC_BAD_FILE = -8,
	E_TLIBC_EOF = -9,	
	E_TLIBC_EMPTY = -10,
	E_TLIBC_INTEGER_OVERFLOW = -11,
	E_TLIBC_IGNORE = -12,
	E_TLIBC_PLEASE_READ_ENUM_NAME = -13,
	E_TLIBC_FILE_IS_ALREADY_ON_THE_STACK = -14,
}tlibc_error_code_t;

#define TLIBC_ERROR_CODE_NUM 15

#ifdef  __cplusplus
}
#endif


#endif
