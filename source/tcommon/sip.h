#ifndef _H_SIP_H
#define _H_SIP_H
/*
* The Socket Interface Protocol
*/

#include <stdint.h>

#pragma pack(push,1)
typedef uint32_t sip_size_t;
typedef struct _sip_cid_t
{
    uint64_t sn;
    uint32_t id;
}sip_cid_t;


typedef enum _sip_req_cmd_t
{
	e_sip_req_cmd_bein = 1,
	e_sip_req_cmd_connect = 2,
	e_sip_req_cmd_recv = 3,
    e_sip_req_cmd_end = 4,
}sip_req_cmd_t;

/*请求=  固定长度的包头 +  包体*/
typedef struct _sip_req_t
{
	sip_req_cmd_t	cmd;            //指令
	sip_cid_t       cid;            //连接id
	sip_size_t      size;           //包体长度
}sip_req_t;

typedef enum _sip_rsp_cmd_t
{
	e_sip_rsp_cmd_accept = 3,
	e_sip_rsp_cmd_send = 4,
	e_sip_rsp_cmd_close = 5,
}sip_rsp_cmd_t;

/*回复= 不定长度的包头 + 包体*/
#define SIP_BROADCAST_NUM 65536
typedef struct _sip_rsp_t
{
	sip_rsp_cmd_t		cmd;                                //指令
	uint32_t            cid_list_num;                       //连接id个数
	sip_cid_t           cid_list[SIP_BROADCAST_NUM];        //连接id数组
    sip_size_t         size;                               //包体长度
}sip_rsp_t;
#pragma pack(pop)


#endif//_H_SIP_H
