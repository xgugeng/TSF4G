#tdata包处理服务器接口协议, 使用共享内存进行传输， 传输内容包括tcp连接的建立， 关闭， 数据接受， 发送等内容。


enum tdgi_cmd_t
{
	e_tdgi_cmd_how_are_you = 0,
	e_tdgi_cmd_fine_thank_you_and_you = 1,	
	e_tdgi_cmd_new_connection_req = 3,
	e_tdgi_cmd_new_connection_ack = 4,

	e_tdgi_cmd_send = 5,
};

union tdgi_body_t(tdgi_cmd_t selector)
{
e_tdgi_cmd_send:
    uint16 send_size;
};


struct tdgi_t
{
	tdgi_cmd_t		cmd;
	uint64          mid;
	tdgi_body_t     body(cmd);
};

