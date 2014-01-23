#tdata包处理服务器接口协议, 使用共享内存进行传输， 传输内容包括tcp连接的建立， 关闭， 数据接受， 发送等内容。


enum tdgi_cmd_t
{
	e_tdgi_cmd_new_connection_req = 0,
};

struct tdgi_new_connection_t
{
	uint32	mid;
};

union tdgi_body_t(tdgi_cmd_t selector)
{
e_tdgi_cmd_new_connection_req:
	tdgi_new_connection_t	new_connection;
};

struct tdgi_t
{
	tdgi_cmd_t		cmd;
	tdgi_body_t		body(cmd);
};

