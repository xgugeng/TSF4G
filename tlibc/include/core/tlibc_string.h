#ifndef _H_TLIBC_STRING
#define _H_TLIBC_STRING

#ifdef  __cplusplus
extern "C" {
#endif


#include "platform/tlibc_platform.h"
#include "core/tlibc_error_code.h"

const char *tstrerror(tlibc_error_code_t terrno);

#ifdef  __cplusplus
}
#endif


#endif//_H_TLIBC_ERRNO

