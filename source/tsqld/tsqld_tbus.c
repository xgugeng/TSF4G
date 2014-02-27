#include "tsqld/tsqld_tbus.h"
#include "tlog/tlog_print.h"
#include "tlog/tlog_log.h"
#include "tsqld_protocol/tsqld_protocol_writer.h"
#include "tsqld_protocol/tsqld_protocol_reader.h"

#include "tlibc/protocol/tlibc_binary_writer.h"
#include "tlibc/protocol/tlibc_binary_reader.h"

#include "tbus/tbus.h"
#include "tcommon/terrno.h"
#include <string.h>
#include <assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "tsqld/tsqld.h"
#include <errno.h>
#include "tlibc/core/tlibc_list.h"

#include "errmsg.h"

tbus_t *g_itb;
tbus_t *g_otb;

static char *g_write_start = NULL;           //开始写入的指针
static char *g_write_limit = NULL;           //结束写入的指针
static tbus_atomic_size_t g_write_size = 0;  //总共可以写入的长度
static char *g_write_cur = NULL;             //当前写入的指针

#define WRITE_LIMIT 1 * 1000000    //最大缓存长度

TERROR_CODE tsqld_tbus_init()
{
    TERROR_CODE ret;
    int input_tbusid;
    int output_tbusid;

	input_tbusid = shmget(g_config.input_tbuskey, 0, 0666);
	if(input_tbusid == -1)
	{
	    ERROR_LOG("shmget errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}
	g_itb = shmat(input_tbusid, NULL, 0);
	if(g_itb == NULL)
	{
        ERROR_LOG("shmat errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}

	output_tbusid = shmget(g_config.output_tbuskey, 0, 0666);
	if(output_tbusid == -1)
	{
        ERROR_LOG("shmget errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}
	g_otb = shmat(output_tbusid, NULL, 0);
	if(g_otb == NULL)
	{
        ERROR_LOG("shmat errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}

    g_write_start = NULL;
    g_write_limit = NULL;
    g_write_size = 0;
    g_write_cur = NULL;

    return E_TS_NOERROR;
shmdt_input:
    shmdt(g_itb);
done:
    return ret;
}

void tsqld_tbus_fini()
{
    shmdt(g_itb);
    shmdt(g_otb);
}

void tsqld_tbus_send(const tsqld_protocol_t *head, const char* data, size_t data_size)
{
    tbus_atomic_size_t total_size= 0;
    size_t head_size;
    char head_buff[sizeof(tsqld_protocol_t)];    
    TLIBC_BINARY_WRITER bw;
    TLIBC_ERROR_CODE r;
    
    tlibc_binary_writer_init(&bw, head_buff, sizeof(head_buff));
    r = tlibc_write_tsqld_protocol_t(&bw.super, head);
    if(r != E_TLIBC_NOERROR)
    {
        ERROR_LOG("tlibc_write_tsqld_protocol_t reutrn [%d].", r);
        goto done;
    }
    

    head_size = bw.offset;
    total_size = (tbus_atomic_size_t)(head_size + data_size);
      

    if(g_write_size < total_size)
    {
        g_write_size = tbus_send_begin(g_otb, &g_write_start);
        g_write_limit = g_write_start + g_write_size;
        g_write_cur = g_write_start;
        
        if(g_write_size < total_size)
        {
            ERROR_LOG("tbus no space, drop mid [%d] data_size [%u].", head->message_id, data_size);
            return;
        }
    }



    memcpy(g_write_cur, head_buff, head_size);         
    g_write_cur += head_size;
    if(data)
    {
        memcpy(g_write_cur, data, data_size);
        g_write_cur += data_size;
    }
    
    if(g_write_cur - g_write_start >= WRITE_LIMIT)
    {
        tbus_send_end(g_otb, (tbus_atomic_size_t)(g_write_cur - g_write_start));
        g_write_size -= (tbus_atomic_size_t)(g_write_cur - g_write_start);
    }

done:
    return;
}

void tsqld_tbus_flush()
{
    if(g_write_size != 0)
    {
        tbus_send_end(g_otb, (tbus_atomic_size_t)(g_write_cur - g_write_start));
        g_write_size = 0;
    }
}
#define TSQLD_MAX_BIND_NUM 1024

static void tsqld_tbus_on_tsqld_query(const tsqld_query_req_s *requery)
{
    const sql_hash_table_s *sql = NULL;
    int r;
//    MYSQL_RES *res = NULL;
//    unsigned int field_num;
//    MYSQL_FIELD *field_vec;
    MYSQL_BIND   par_bind[TSQLD_MAX_BIND_NUM];
    MYSQL_BIND   res_bind[TSQLD_MAX_BIND_NUM];
    my_bool     res_null[TSQLD_MAX_BIND_NUM];
    char*str = "haha";
    
    int id;
    char username[1024];

    const tlibc_hash_head_t *sql_hash = tlibc_hash_find_const(&g_sql_hash, requery->name, (uint32_t)strlen(requery->name));
    if(sql_hash == NULL)
    {
        goto done;
    }

    sql = TLIBC_CONTAINER_OF(sql_hash, const sql_hash_table_s, entry);

    
    //这里要检查sql->param_count保证数组不会出界
    par_bind[0].buffer_type = MYSQL_TYPE_STRING;
    par_bind[0].buffer_length = strlen(str);
    par_bind[0].buffer = str;
    par_bind[0].is_null = 0;
    par_bind[0].length = 0;
    //实数类型的编码在mysql中需要特别当心


    //bind的程度由sql语句决定
    r = mysql_stmt_bind_param(sql->stmt, par_bind);
    if(r)
    {
        ERROR_LOG("mysql_stmt_bind_param Error %u: %s", mysql_errno(g_mysql), mysql_error(g_mysql));
        exit(1);
    }
    
    r = mysql_stmt_execute(sql->stmt);
    if(r != 0)
    {
        ERROR_LOG("mysql_real_query Error %u: %s", mysql_errno(g_mysql), mysql_error(g_mysql));
        exit(1);
    }

    
    if(sql->res)
    {
        id = 0;
        unsigned long len;
        assert(sql->field_vec != 0);
        //当为长度类型时候不需要设置buffer_length
        res_bind[0].buffer_type = sql->field_vec[0].type;
        res_bind[0].length = 0;
        res_bind[0].buffer= &id;
        res_bind[0].is_null = &res_null[0];


        res_bind[1].buffer_type = sql->field_vec[1].type;
        len = 1;//sizeof(username);
        //这个类型要和mysql头文件中的一致， 不然会出错
        assert(sizeof(res_bind[1].length) == sizeof(len));
        res_bind[1].buffer_length = len;
        //返回str的长度
        res_bind[1].length = &len;
        res_bind[1].buffer= username;
        res_bind[1].is_null = &res_null[1];

        //值的实际大小， 超过缓存值
        if(len >= res_bind[1].buffer_length)
        {
            ERROR_LOG("not enough memory.");
        }

        //这里要检查返回结果的列， 以保证不会出错
//        sql->field_count
        

        r = mysql_stmt_bind_result(sql->stmt, res_bind);
        if(r)
        {
            ERROR_LOG("mysql_stmt_bind_result Error %u: %s", mysql_errno(g_mysql), mysql_error(g_mysql));
            exit(1);
        }

        r = mysql_stmt_store_result(sql->stmt);
        if(r)
        {
            ERROR_LOG("mysql_stmt_store_result Error %u: %s", mysql_errno(g_mysql), mysql_error(g_mysql));
            exit(1);
        }

        while (mysql_stmt_fetch(sql->stmt) != MYSQL_NO_DATA)
        {
            printf("%d", id);
        }
    }
    INFO_PRINT(sql->sql);
done:
    return;
}

static void tsqld_tbus_pkg(const tsqld_protocol_t *req)
{
    switch(req->message_id)
    {
    case e_tsqld_query_req:
        tsqld_tbus_on_tsqld_query(&req->body.query_req);
        break;
	default:
		break;
    }
}
tsqld_protocol_t head;

TERROR_CODE tsqld_tbus_proc()
{
    TERROR_CODE ret = E_TS_NOERROR;
    char*message, *cur, *message_limit;
    tbus_atomic_size_t message_len = 0;
        
    message_len = tbus_read_begin(g_itb, &message);
    if(message == 0)
    {
        ret = E_TS_WOULD_BLOCK;
        goto done;
    }

    message_limit = message + message_len;

    for(cur = message; cur < message_limit;)
    {
        TLIBC_BINARY_READER br;
        TLIBC_ERROR_CODE r;

        
        tlibc_binary_reader_init(&br, cur, (uint32_t)(message_limit - cur));
        r = tlibc_read_tsqld_protocol_t(&br.super, &head);
        if(r != E_TLIBC_NOERROR)
        {
            ERROR_LOG("tlibc_write_tsqld_protocol_t reutrn [%d].", r);
            goto read_end;
        }
        tsqld_tbus_pkg(&head);
        cur += br.offset;
    }

    assert(cur == message_limit);
read_end:
    tbus_read_end(g_itb, message_len);
done:
    return ret;
}

