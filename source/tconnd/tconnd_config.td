import "tcommon/tcommon.td"

const uint16 TCONND_MAX_TDTP_NUM = 32;

#tdata包处理服务器接口协议, 使用共享内存进行传输， 传输内容包括tcp连接的建立， 关闭， 数据接受， 发送等内容。
struct tconnd_tdgi_t
{
	int32 shmkey;
};

#tdata包传输协议， 使用tcp进行传输， 包含包头， 包体两部分。 包头是一个小端编码的16位整数， 表示包体的大小。
struct tconnd_tdtp_t
{
	string<TSERVER_IP_LENGTH> ip;
	uint16 port;
	uint32 backlog;

	uint32 epoll_size;

	tconnd_tdgi_t tdgi_pass;

	int32 input_tbuskey;
	int32 output_tbuskey;
};

struct tconnd_config_t
{
	string<TSERVER_FILE_NAME_LENGH>		log_config;
	vector<tconnd_tdtp_t, TCONND_MAX_TDTP_NUM> tdtp;
};

