#include "tlogd.h"
#include "tapp.h"
#include "terrno.h"
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

#define MAX_BIND_NUM 65536
#define EXECUTE_NUM_LIMIT 4096
#define MYINIT_INTERVAL_S 5

tlogd_config_t           g_config;

MYSQL                   *g_conn = NULL;
MYSQL_STMT              *g_stmt = NULL;
MYSQL_BIND               g_bind[MAX_BIND_NUM]; 
tlog_message_t           g_message[EXECUTE_NUM_LIMIT];
size_t				     g_message_num;
int                      g_input_tbus_id;
tbus_t                  *g_input_tbus;
tlibc_binary_reader_t    g_binary_reader;
tlibc_mybind_reader_t    g_mybind_reader;

bool					 g_myinited;
time_t					 g_last_myinit;

static TERROR_CODE init()
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

	g_myinited = false;
	g_last_myinit = 0;
	return E_TS_NOERROR;
ERROR_RET:
	return E_TS_ERROR;
}

static void fini()
{
	shmdt(g_input_tbus);
}


static TERROR_CODE myinit()
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

	g_message_num = 0;

    return E_TS_NOERROR;
close_stmt:
    mysql_stmt_close(g_stmt);
close_mysql:
    mysql_close(g_conn);
error_ret:
    return E_TS_ERROR;
}

static void myfini()
{
    mysql_stmt_close(g_stmt);
    mysql_close(g_conn);

    return;
}

static TERROR_CODE process()
{
    TERROR_CODE ret = E_TS_NOERROR;
    char *ptr;
	if(!g_myinited)
	{
		if(myinit() == E_TS_NOERROR)
		{
			g_myinited = true;
		}
		else
		{
			ret =  E_TS_WOULD_BLOCK;
			goto done;
		}
	}

    g_binary_reader.size = tbus_read_begin(g_input_tbus, &ptr);
    g_binary_reader.addr = ptr;
    if(g_binary_reader.size == 0)
    {
        ret = E_TS_WOULD_BLOCK;
		if(g_message_num > 0)
		{
			if(mysql_commit(g_conn))
			{
                ERROR_PRINT("mysql_commit failed.");
				myfini();
				g_myinited = false;
				goto done;
			}
			g_message_num = 0;
		}
        goto done;
    }
    
    for(g_binary_reader.offset = 0; g_binary_reader.offset < g_binary_reader.size; )
    {    
        if(tlibc_read_tlog_message(&g_binary_reader.super, &g_message[g_message_num]) == E_TS_NOERROR)
        {
    		tlibc_error_code_t    r;
   			tlibc_mybind_reader_init(&g_mybind_reader, g_bind, MAX_BIND_NUM);
			r = tlibc_read_tlog_message(&g_mybind_reader.super, &g_message[g_message_num]);
			if(r != E_TLIBC_NOERROR)
			{
				ERROR_PRINT("tlibc_read_tlog_message(), errno %d", r);
				ret = E_TS_ERROR;
				goto done;
			}
            if (mysql_stmt_bind_param(g_stmt, g_bind))
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
                goto done;
            }
			++g_message_num;
        }
		else
		{
			ERROR_PRINT("bad packet.");
            ret = E_TS_ERROR;
            goto done;
		}
    }

    if(g_binary_reader.size != g_binary_reader.offset)
    {
		ERROR_PRINT("bad packet.");
        ret = E_TS_ERROR;
        goto done;
    }
    
	if(g_message_num >= EXECUTE_NUM_LIMIT)
	{
		if(mysql_commit(g_conn))
		{
			ERROR_PRINT("mysql_commit failed.");
			myfini();
			g_myinited = false;
			goto done;
		}
		g_message_num = 0;
	}

    tbus_read_end(g_input_tbus, g_binary_reader.size);
done:
	return ret;
}


int main(int argc, char **argv)
{
    int ret = 0;
    
    tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tlogd_config);
	if(init() != E_TS_NOERROR)
	{
		ret = 1;
		goto done;
	}

	if(tapp_loop(process, TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL) != E_TS_NOERROR)
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

