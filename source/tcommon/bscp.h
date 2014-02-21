#ifndef _H_BSCP_H
#define _H_BSCP_H

/*
* The binary stream cutting protocol.
* 字节流切割协议
*
* 协议= 包头 +  包体
* 包头= 为16位或32位或64位小端编码的正整数， 表示包体的长度。
*
*/

#include <stdint.h>

typedef uint16_t bscp16_head_t;

typedef uint32_t bscp32_head_t;

typedef uint64_t bscp64_head_t;

#endif//_H_BSCP_H

