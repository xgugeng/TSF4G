import "tcommon.td"

const uint32 TSERVER_DBNAME_LENGTH = 16;
const uint32 TSERVER_SQL_LENGTH = 1024;

struct tlogd_config
{
	int32								input_tbuskey;					#接收log所用的tbuskey, 协议为2字节包头表示包体长度 + 包体
	string<TSERVER_IP_LENGTH> 			ip;								#数据库ip
	uint16 								port;							#数据库端口
	string<TSERVER_DBNAME_LENGTH>		dbname;							#数据库
	string<TSERVER_DBNAME_LENGTH>		user;							#数据库用户名
	string<TSERVER_DBNAME_LENGTH>		password;						#数据库密码
	string<TSERVER_SQL_LENGTH>			sql;							#存储log时执行的sql语句
};

