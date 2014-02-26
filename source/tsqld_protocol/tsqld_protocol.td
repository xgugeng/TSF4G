const uint32 TSQLD_NAME_LENGTH = 1024;

enum tsqld_message_id
{
	e_tsqld_query_req = 1000,
	e_tsqld_query_rsp = 1001,
};

struct tsqld_query_req_s
{
	uint64					  sn;
	string<TSQLD_NAME_LENGTH> name;
};

const uint32 TSQLD_RESULT_LENGTH = 10000000;#10m的最大返回值
struct tsqld_query_rsp_s
{
	uint64											sn;
	vector<char, TSQLD_RESULT_LENGTH>				result;
};

union tsqld_message_body_u(tsqld_message_id selector)
{
e_tsqld_query_req:	
	tsqld_query_req_s query_req;
e_tsqld_query_rsp:	
	tsqld_query_req_s query_rsp;
};

struct tsqld_protocol_t
{
	tsqld_message_id message_id;
	tsqld_message_body_u body(message_id);
};

