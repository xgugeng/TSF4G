#ifndef _H_BSCP_H
#define _H_BSCP_H

/*
* The binary stream cutting protocol.
* 字节流切割协议
*
* 协议= 包头 +  包体
* 包头= 为小端编码的正整数， 表示包体的长度。
*
*/

#include <stdint.h>
#include "tlibc/core/tlibc_util.h"

typedef uint16_t bscp_head_t;

#define bscp_head_t_code(head) {tlibc_host16_to_little(head);}

#define bscp_head_t_decode(head) {tlibc_little_to_host16(head);}


#endif//_H_BSCP_H

