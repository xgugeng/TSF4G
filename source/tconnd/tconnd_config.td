import "tcommon/tcommon.td"

#tdgi: tdata包处理服务器接口协议, 使用共享内存进行传输， 传输内容包括tcp连接的建立， 关闭， 数据接受， 发送等内容。
#tdtp: tdata包传输协议， 使用tcp进行传输， 包含包头， 包体两部分。 包头是一个小端编码的16位整数， 表示包体的大小。


struct tconnd_config_t
{
	string<TSERVER_FILE_NAME_LENGH>		log_config; 	#log配置文件的路径
	string<TSERVER_IP_LENGTH> 			ip;				#监听ip
	uint16 								port;			#监听端口
	uint32 								backlog;		#未处理的连接数
	uint32 								connections;	#最多可以接入的连接数
	int32 								tdgi_pass;		#转发的tdgi服务器
	int32 								input_tbuskey;	#输入tbuskey
	int32								output_tbuskey; #输出tbuskey
	int32                               defer_accept;   #TCP_DEFER_ACCEPT
	uint32								sndbuf;			#SO_SNDBUF
	uint32								rcvbuf;			#SO_RCVBUF
	uint32								nodelay;		#TCP_NODELAY
	uint32								cork;			#TCP_CORK
	uint32								quickack;		#TCP_QUICKACK
	uint32								keepalive;		#SO_KEEPALIVE
if(keepalive == 1)
	uint32								keepidle;		#TCP_KEEPIDLE
if(keepalive == 1)
	uint32								keepintvl;		#TCP_KEEPINTVL
if(keepalive == 1)
	uint32								keepcnt;		#TCP_KEEPCNT
};

