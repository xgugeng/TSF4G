import "tsqld_protocol/tsqld_protocol.td"


const uint32 TSQLD_SQL_NUM = 65536;
const uint32 TSQLD_SQL_LENGTH = 2048;

struct tsqld_sql_vec
{
	string<TSQLD_NAME_LENGTH> name;
	string<TSQLD_SQL_LENGTH> sql;
};

struct tsqld_config_t
{
    string<TSQLD_NAME_LENGTH>                                   host;
    uint32                                                      port;
    string<TSQLD_NAME_LENGTH>                                   user;
    string<TSQLD_NAME_LENGTH>                                   password;
    string<TSQLD_NAME_LENGTH>                                   db;
	int32 														input_tbuskey;					#输入tbuskey
	int32														output_tbuskey; 				#输出tbuskey
	vector<tsqld_sql_vec, TSQLD_SQL_NUM>						sql_vec;
};

