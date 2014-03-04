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
#include "bscp_types.h"

#define bscp_head_t_code(head) {tlibc_host16_to_little(head);}

#define bscp_head_t_decode(head) {tlibc_little_to_host16(head);}

#define BSCP_HEAD_T_SIZE sizeof(bscp_head_t)


#endif//_H_BSCP_H

