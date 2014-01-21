#ifndef _H_TERRNO_H
#define _H_TERRNO_H

typedef enum _TERROR_CODE
{
	E_TS_NOERROR = 0,
	E_TS_ERROR = 1,

	E_TS_NO_MEMORY = 2,
	E_TS_AGAIN = 3,
	E_TS_WOULD_BLOCK = 4,

	E_TS_CAN_NOT_OPEN_FILE = 5,

}TERROR_CODE;

#endif

