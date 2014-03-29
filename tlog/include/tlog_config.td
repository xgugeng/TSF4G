import "tcommon.td"
import "tlog_sql.td"

const uint16 TLOG_MAX_APPENDER_NUM = 32;

enum tlog_level_t
{
	e_tlog_error = 1,
	e_tlog_warn = 2,
	e_tlog_info = 3,
	e_tlog_debug = 4,
};

enum tlog_appender_type_t
{
	e_tlog_appender_rolling_file = 0,
    e_tlog_appender_shm = 1,
};






struct tlog_config_appender_rolling_file_t
{
	string<TSERVER_FILE_NAME_LENGH> file_name;
	uint32							max_file_size;
	uint32							max_backup_index;
};

struct tlog_config_appender_shm_t
{
	int32                           output_tbuskey;
};

union tlog_config_appender_body(tlog_appender_type_t selector)
{
e_tlog_appender_rolling_file: tlog_config_appender_rolling_file_t rolling_file;
e_tlog_appender_shm: tlog_config_appender_shm_t shm;
};

struct tlog_config_appender_t
{
	tlog_appender_type_t type;
	tlog_config_appender_body appender(type);
};

struct tlog_config_t
{
	tlog_level_t level;
	vector<tlog_config_appender_t, TLOG_MAX_APPENDER_NUM> appender_vec;
};

