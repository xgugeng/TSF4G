import "tcommon/tcommon.td"

const uint32 TSQLD_SQL_NUM = 65536;
const uint32 TSQLD_SQL_LENGTH = 2048;
const uint32 TSQLD_NAME_LENGTH = 1024;

struct tsqld_sql_vec
{
	string<TSQLD_NAME_LENGTH> name;
	string<TSQLD_SQL_LENGTH> sql;
};

struct tsqld_config_t
{
	int32 														input_tbuskey;					#输入tbuskey
	int32														output_tbuskey; 				#输出tbuskey
	vector<tsqld_sql_vec, TSQLD_SQL_NUM>						sql_vec;
};

