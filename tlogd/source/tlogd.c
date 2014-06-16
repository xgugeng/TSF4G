#include "tlogd.h"
#include "tapp.h"
#include "tlibc_error_code.h"
#include "tlog_print.h"
#include "tbus.h"

#include "tlogd_config_types.h"
#include "tlogd_config_reader.h"

#include "tlog_sql_types.h"
#include "tlog_sql_writer.h"
#include "tlog_sql_reader.h"

#include "protocol/tlibc_mybind_reader.h"
#include "protocol/tlibc_binary_reader.h"



#include "mysql.h"

#include <string.h>
#include <stdio.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

#define TLOGD_IOV_NUM 65536
#define MAX_BIND_NUM 65536
#define MYINIT_INTERVAL_S 5

tlogd_config_t           g_config;

MYSQL                   *g_conn = NULL;
MYSQL_STMT              *g_stmt = NULL;
MYSQL_BIND               g_bind[MAX_BIND_NUM]; 
tlog_message_t           g_message;
int                      g_input_tbus_id;
tbus_t                  *g_input_tbus;
tlibc_binary_reader_t    g_binary_reader;
tlibc_mybind_reader_t    g_mybind_reader;

bool					 g_myinited;
time_t					 g_last_myinit;

static tlibc_error_code_t init()
{
    g_input_tbus_id = shmget(g_config.input_tbuskey, 0, 0666);
    if(g_input_tbus_id == -1)
    {
        ERROR_PRINT("shmget returned an errno[%d], %s.", errno, strerror(errno));
        goto ERROR_RET;
    }
	g_input_tbus = shmat(g_input_tbus_id, NULL, 0);
	if(g_input_tbus == NULL)
	{
        ERROR_PRINT("shmat returned an errno[%d], %s.", errno, strerror(errno));
        goto ERROR_RET;
	}

    tlibc_binary_reader_init(&g_binary_reader, NULL, 0);
   	tlibc_mybind_reader_init(&g_mybind_reader, g_bind, MAX_BIND_NUM);

	g_myinited = false;
	g_last_myinit = 0;
	return E_TLIBC_NOERROR;
ERROR_RET:
	return E_TLIBC_ERROR;
}

static void fini()
{
	shmdt(g_input_tbus);
}


static tlibc_error_code_t myinit()
{
    char                *password = NULL;
	time_t				current_time = time(NULL);

	if(current_time - g_last_myinit < MYINIT_INTERVAL_S)
	{
		goto error_ret;
	}
	g_last_myinit = current_time; 
    
    g_conn = mysql_init(NULL);
    if(g_conn == NULL)
    {
        ERROR_PRINT("mysql_init(), error %s", mysql_error(g_conn)); 
        goto error_ret;
    }

    if(g_config.password[0])
    {
        password = g_config.password;
    }
    else
    {

        password = NULL;
    }
    
    if(!mysql_real_connect(g_conn, g_config.ip, g_config.user, password
        , g_config.dbname, g_config.port, NULL,0))
    {
        ERROR_PRINT("mysql_real_connect(), error %s", mysql_error(g_conn)); 
        goto close_mysql;
    }

    g_stmt = mysql_stmt_init(g_conn);
    if(g_stmt == NULL)
    {
        ERROR_PRINT("mysql_stmt_init(), error %s", mysql_error(g_conn)); 
        goto close_mysql;
    }

    if(mysql_stmt_prepare(g_stmt, g_config.sql, strlen(g_config.sql)))
    {
        ERROR_PRINT("mysql_stmt_prepare(), error %s", mysql_error(g_conn)); 
        goto close_stmt;
    }

    if(mysql_autocommit(g_conn, false))
    {
        ERROR_PRINT("mysql_autocommit(g_conn, false), error %s", mysql_error(g_conn)); 

        goto close_stmt;
    }
	INFO_PRINT("myinit succeed.");

    return E_TLIBC_NOERROR;
close_stmt:
    mysql_stmt_close(g_stmt);
close_mysql:
    mysql_close(g_conn);
error_ret:
    return E_TLIBC_ERROR;
}

static void myfini()
{
    mysql_stmt_close(g_stmt);
    mysql_close(g_conn);

    return;
}


static tlibc_error_code_t process(void *arg)
{
    tlibc_error_code_t ret = E_TLIBC_NOERROR;
	tbus_atomic_size_t tbus_head;
	struct iovec iov[TLOGD_IOV_NUM];
	size_t iov_num;
	size_t i;

	if(!g_myinited)
	{
		if(myinit() == E_TLIBC_NOERROR)
		{
			g_myinited = true;
		}
		else
		{
			ret =  E_TLIBC_WOULD_BLOCK;
			goto done;
		}
	}

	iov_num = TLOGD_IOV_NUM;
	tbus_head = tbus_read_begin(g_input_tbus, iov, &iov_num);	
	if(iov_num == 0)
	{
	    if(tbus_head != g_input_tbus->head_offset)
	    {
	        goto read_end;
	    }
	    else
	    {
	        ret = E_TLIBC_WOULD_BLOCK;
	        goto done;
	    }
	}

	for(i = 0;i < iov_num;++i)
	{
    	tlibc_error_code_t    r;

		g_binary_reader.offset = 0;
		g_binary_reader.size = (uint32_t)iov[i].iov_len;
		g_binary_reader.addr = iov[i].iov_base;
		r = tlibc_read_tlog_message(&g_binary_reader.super, &g_message);
    
        if(r != E_TLIBC_NOERROR)
		{
			ERROR_PRINT("tlibc_read_tlog_message(), errno %d", r);
			goto commit;
		}

		g_mybind_reader.idx = 0;
		r = tlibc_read_tlog_message(&g_mybind_reader.super, &g_message);
		if(r != E_TLIBC_NOERROR)
		{
			ERROR_PRINT("tlibc_read_tlog_message(), errno %d", r);
			goto commit;
		}

		if(mysql_stmt_bind_param(g_stmt, g_bind))
		{
			ERROR_PRINT("mysql_stmt_bind_param(), error %s", mysql_error(g_conn)); 
			myfini();
			g_myinited = false;
			goto done;
		}
            
        if(mysql_stmt_execute(g_stmt))
        {
            ERROR_PRINT("mysql_stmt_execute(), error %s", mysql_stmt_error(g_stmt));
			myfini();
			g_myinited = false;
            goto read_end;
        }
    }

commit:
	if(mysql_commit(g_conn))
	{
		ERROR_PRINT("mysql_commit failed.");
		myfini();
		g_myinited = false;
		goto read_end;
	}

read_end:
    tbus_read_end(g_input_tbus, tbus_head);
done:
	return ret;
}


int main(int argc, char **argv)
{
    int ret = 0;
    
    tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tlogd_config);
	if(init() != E_TLIBC_NOERROR)
	{
		ret = 1;
		goto done;
	}

	if(tapp_loop(TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL, NULL, NULL
	             , process, NULL
	             , NULL, NULL) != E_TLIBC_NOERROR)
	{
		ret = 1;
	}

	if(g_myinited)
	{
		myfini();
	}

	fini();
done:
    return ret;
}

