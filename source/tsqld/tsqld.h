#ifndef _H_TSQLD_H
#define _H_TSQLD_H

#include <stdbool.h>
#include "tsqld/tsqld_config_types.h"
#include "mysql.h"
#include "tlibc/core/tlibc_hash.h"

extern tsqld_config_t g_config;
extern bool g_sig_term;

typedef struct _sql_hash_table_s
{
    tlibc_hash_head_t entry;
    const tsqld_sql_vec* sql;
}sql_hash_table_s;

extern sql_hash_table_s g_sql_hash_table[TSQLD_SQL_NUM];
extern tlibc_hash_t g_sql_hash;
extern MYSQL g_mysql;







#endif//_H_TSQLD_H

