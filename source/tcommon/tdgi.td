#tdata包处理服务器接口协议, 使用共享内存进行传输， 传输内容包括tcp连接的建立， 关闭， 数据接受， 发送等内容。

enum tdgi_cmd_t
{
	e_tdgi_cmd_new_connection_req,
	e_tdgi_cmd_new_connection_ack,

	e_tdgi_cmd_send,
	e_tdgi_cmd_recv,
};

const uint32 TDGI_BROADCAST_LIMIT = 65536;

struct tdgi_t
{
	tdgi_cmd_t		cmd;
	vector<uint64, TDGI_BROADCAST_LIMIT>  mid;
	uint16          size;
};

