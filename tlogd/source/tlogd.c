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



#define MAX_BIND_NUM 65536

tlogd_config_t          g_config;

MYSQL                   *g_conn = NULL;
MYSQL_STMT              *g_stmt = NULL;
tlog_message_t           g_message;
int                      g_input_tbus_id;
tbus_t                  *g_input_tbus;
tlibc_binary_reader_t      g_binary_reader;
MYSQL_BIND               g_bind[MAX_BIND_NUM]; 
tlibc_mybind_reader_t      g_mybind_reader;



static TERROR_CODE init()
{
    char                *password = NULL;
    
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

    g_conn = mysql_init(NULL);
    if(g_conn == NULL)
    {
        ERROR_PRINT("mysql_init(), error %s\n", mysql_error(g_conn)); 
        goto shmdt_m;
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
        ERROR_PRINT("mysql_real_connect(), error %s\n", mysql_error(g_conn)); 
        goto close_mysql_m;
    }

    g_stmt = mysql_stmt_init(g_conn);
    if(g_stmt == NULL)
    {
        ERROR_PRINT("mysql_stmt_init(), error %s\n", mysql_error(g_conn)); 
        goto close_mysql_m;
    }

    if(mysql_stmt_prepare(g_stmt, g_config.sql, strlen(g_config.sql)))
    {
        ERROR_PRINT("mysql_stmt_prepare(), error %s\n", mysql_error(g_conn)); 
        goto close_stmt_m;
    }

    if(mysql_autocommit(g_conn, false))
    {
        ERROR_PRINT("mysql_autocommit(g_conn, false), error %s\n", mysql_error(g_conn)); 

        goto close_stmt_m;
    }

    tlibc_mybind_reader_init(&g_mybind_reader, g_bind, MAX_BIND_NUM);
    tlibc_binary_reader_init(&g_binary_reader, NULL, 0);

    return E_TS_NOERROR;
close_stmt_m:
    mysql_stmt_close(g_stmt);
close_mysql_m:
    mysql_close(g_conn);
shmdt_m:
    shmdt(g_input_tbus);
ERROR_RET:
    return E_TS_ERROR;
}

static TERROR_CODE process()
{
    TERROR_CODE ret = E_TS_NOERROR;
    char *ptr;

    g_binary_reader.size = tbus_read_begin(g_input_tbus, &ptr);
    g_binary_reader.addr = ptr;
    if(g_binary_reader.size == 0)
    {
        ret = E_TS_WOULD_BLOCK;
        goto done;
    }
    
    for(g_binary_reader.offset = 0; g_binary_reader.offset < g_binary_reader.size; )
    {    
        if(tlibc_read_tlog_message(&g_binary_reader.super, &g_message) == E_TS_NOERROR)
        {
            tlibc_error_code_t    r;
            r = tlibc_read_tlog_message(&g_mybind_reader.super, &g_message);
            if(r != E_TLIBC_NOERROR)
            {
                ERROR_PRINT("tlibc_write_tlog_message_t(), errno %d\n", r);
                ret = E_TS_ERROR;
                goto done;
            }

            if (mysql_stmt_bind_param(g_stmt, g_bind))
            {
                ERROR_PRINT("mysql_stmt_bind_param(), error %s\n", mysql_error(g_conn)); 
                ret = E_TS_ERROR;
                goto done;
            }
            
            if(mysql_stmt_execute(g_stmt))
            {
                ERROR_PRINT("mysql_stmt_execute(), error %s", mysql_stmt_error(g_stmt));
                ret = E_TS_ERROR;
                goto done;
            }
        }
    }

    if(g_binary_reader.size != g_binary_reader.offset)
    {
        ret = E_TS_ERROR;
        goto done;
    }
    
    if(mysql_commit(g_conn))
    {
        ret = E_TS_ERROR;
        goto done;
    }
    
    tbus_read_end(g_input_tbus, g_binary_reader.size);

done:
    return ret;
}

static void fini()
{
    mysql_stmt_close(g_stmt);
    mysql_close(g_conn);

    return;
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
        goto done;
    }

    fini();
done:
    return ret;
}

