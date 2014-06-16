#ifndef _H_TERRNO_H
#define _H_TERRNO_H

#ifdef  __cplusplus
extern "C" {
#endif

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

typedef enum _TERROR_CODE
{
	E_TS_NOERROR = 0,
	E_TS_ERROR = 1,

	E_TS_WOULD_BLOCK = 2,
    E_TS_ERRNO = 3,
	E_TS_NO_MEMORY = 4,
	E_TS_CAN_NOT_OPEN_FILE = 5,
	E_TS_CLOSE = 6,
    E_TS_TBUS_NOT_ENOUGH_SPACE = 7,
    E_TS_TOO_MANY_SOCKET = 8,
    E_TS_BAD_PACKAGE = 9,
	E_TS_MYSQL_ERROR = 10,
}TERROR_CODE;

#ifdef  __cplusplus
}
#endif

#endif


