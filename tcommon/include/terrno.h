#ifndef _H_TERRNO_H
#define _H_TERRNO_H

#ifdef  __cplusplus
extern "C" {
#endif

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


