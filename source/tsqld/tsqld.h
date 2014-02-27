#ifndef _H_TSQLD_H
#define _H_TSQLD_H

#include <stdbool.h>
#include <stdint.h>
#include "tsqld/tsqld_config_types.h"
#include "mysql.h"
#include "tlibc/core/tlibc_hash.h"

extern tsqld_config_t g_config;
extern bool g_sig_term;

typedef struct _sql_hash_table_s
{
    tlibc_hash_head_t entry;
    MYSQL_STMT *stmt;
    uint32_t param_count;
        
    MYSQL_RES *res;
    MYSQL_FIELD *field_vec;
    uint32_t field_vec_count;

    const char* sql;
}sql_hash_table_s;

extern sql_hash_table_s g_sql_hash_table[TSQLD_SQL_NUM];
extern tlibc_hash_t g_sql_hash;
extern MYSQL *g_mysql;







#endif//_H_TSQLD_H

