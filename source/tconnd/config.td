const uint16 TCONND_MAX_IP_LENGTH = 16;
const uint16 TCONND_FILE_PATH_LENGTH = 1024;
const uint16 TCONND_MAX_TDTP_NUM = 32;

#tdata包处理服务器接口协议, 使用共享内存进行传输， 传输内容包括tcp连接的建立， 关闭， 数据接受， 发送等内容。
struct tdgi_t
{
	int32 shmkey;
};

#tdata包传输协议， 使用tcp进行传输， 包含包头， 包体两部分。 包头是一个小端编码的16位整数， 表示包体的大小。
struct tdtp_t
{
	string<TCONND_MAX_IP_LENGTH> ip;
	uint16 port;

	tdgi_t tdgi_pass;
};

struct config_t
{
	string<TCONND_FILE_PATH_LENGTH>		log_config;
	vector<tdtp_t, TCONND_MAX_TDTP_NUM> tdtp;
};

