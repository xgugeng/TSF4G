#ifndef _H_SIP_H
#define _H_SIP_H

#ifdef  __cplusplus
extern "C" {
#endif

/*
* The Socket Interface Protocol
* sip使用tbus进行传输， 对socket接口进行了包装。
*/

#include <stdint.h>

/*sn 在回话中不会重复*/
typedef uint32_t sip_size_t;
typedef struct sip_cid_s
{
    uint64_t sn;
    uint32_t id;
}sip_cid_t;


/*
 * req =  固定长度的包头 +  包体
 * tbus一次只发送一个req，server用此特性来简化处理流程。
 */
typedef enum sip_req_cmd_e
{
	e_sip_req_cmd_connect = 1,
	e_sip_req_cmd_recv = 2,
}sip_req_cmd_t;

typedef struct sip_req_s
{
	sip_req_cmd_t	cmd;            //指令
    sip_size_t      size;           //包体长度
	sip_cid_t       cid;            //连接id
}sip_req_t;



/*
 * rsp = 不定长度的包头 + 包体
 * tbus一次发送多个rsp，tconnd用此特性来减少系统调用次数。
 */
typedef enum sip_rsp_cmd_s
{
	e_sip_rsp_cmd_accept = 3,
	e_sip_rsp_cmd_send = 4,
	e_sip_rsp_cmd_close = 5,
}sip_rsp_cmd_t;

#define SIP_BROADCAST_NUM 65535
typedef struct sip_rsp_s
{
	sip_rsp_cmd_t		cmd;                                //指令
    sip_size_t          size;                               //包体长度
	uint16_t            cid_list_num;                       //连接id个数, > 0
	sip_cid_t           cid_list[SIP_BROADCAST_NUM];        //连接id数组
}sip_rsp_t;
#define SIZEOF_SIP_RSP_T(h) (size_t)((const char*)&(h)->cid_list[(h)->cid_list_num] - (const char*)(h))

#ifdef  __cplusplus
}
#endif

#endif//_H_SIP_H
