#ifndef _H_TDTP_H
#define _H_TDTP_H

#include "tcommon/tdtp_types.h"
#include "tlibc/core/tlibc_util.h"

#ifdef TDTP_SIZEOF_SIZE_T
    #if TDTP_SIZEOF_SIZE_T == 2
        #define TDTP_SIZE2LITTLE(x) tlibc_host16_to_little(x)
        #define TDTP_LITTLE2SIZE(x) tlibc_little_to_host16(x)
    #elif TDTP_SIZEOF_SIZE_T == 4
        #define TDTP_SIZE2LITTLE(x) tlibc_host32_to_little(x)
        #define TDTP_LITTLE2SIZE(x) tlibc_little_to_host32(x)
    #elif TDTP_SIZEOF_SIZE_T == 8
        #define TDTP_SIZE2LITTLE(x) tlibc_host64_to_little(x)
        #define TDTP_LITTLE2SIZE(x) tlibc_little_to_host64(x)
    #else
        #error "tdtp_size_t must be 2, 4, 6."
    #endif
#else
    #error "please define TDTP_SIZEOF_SIZE_T"    
#endif


#endif//_H_TDTP_H

