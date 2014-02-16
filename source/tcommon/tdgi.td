#tdata包处理服务器接口协议, 使用共享内存进行传输， 传输内容包括tcp连接的建立， 关闭， 数据接受， 发送等内容。

enum tdgi_cmd_t
{
	e_tdgi_cmd_connect = 1,
	e_tdgi_cmd_recv = 2,
	
	e_tdgi_cmd_accept = 3,
	e_tdgi_cmd_send = 4,
	e_tdgi_cmd_close = 5,
};

const uint32 TDGI_BROADCAST_LIMIT = 65536;
const uint32 TDGI_SIZE_T_MAX = 0xffffffff;
typedef uint32 tdgi_size_t;

struct tdgi_req_t
{
	tdgi_cmd_t		cmd;
	uint64          mid;
	tdgi_size_t     size;
};
#ͷ����СΪ�̶�ֵ
const uint32 TDGI_REQ_HEAD_SIZE = 16;

struct tdgi_rsp_t
{
	tdgi_cmd_t		cmd;
	vector<uint64, TDGI_BROADCAST_LIMIT>  mid;
	tdgi_size_t          size;
};

