#ifndef _H_TCONND_MEMPOOL_H
#define _H_TCONND_MEMPOOL_H

#include "tlibc/platform/tlibc_platform.h"
#include "tcommon/terrno.h"
#include "tlibc/core/tlibc_mempool.h"

#include <stdint.h>
#include <limits.h>


typedef struct _tconnd_mempool_entry_s
{
    uint64_t         sn;
    TLIBC_LIST_HEAD  unused_list;
    TLIBC_LIST_HEAD  used_list;
}tconnd_mempool_entry_s;

typedef struct _tconnd_mempool_s
{
    char                        *pool;
    uint64_t                    sn;
    size_t                      unit_size;
    size_t                      unit_num;
    tconnd_mempool_entry_s      mempool_entry;
    size_t                      unused_list_num;
    size_t                      used_list_num;
}tconnd_mempool_s;

#define tm_invalid_id UINT64_MAX


#define tm_id_test(self, id) (id < (self)->unit_num)
#define tm_id2ptr(self, id) ((self)->pool + (self)->unit_size * id)
#define tm_ptr_test(ptr, entry, sno) (((ptr)->entry.sn != tm_invalid_id) && ((ptr)->entry.sn == sno))

#define tm_ptr2id(self, ptr) (((char*)ptr - (self)->pool) / (self)->unit_size)

#define tm_init(self, type, entry, p, us, un)\
{\
    size_t i;\
    (self)->sn = 0;\
    (self)->pool = (p);\
    (self)->unit_size = (us);\
    (self)->unit_num = (un);\
    tlibc_list_init(&(self)->mempool_entry.unused_list);\
    tlibc_list_init(&(self)->mempool_entry.used_list);\
    (self)->used_list_num = 0;\
    for(i = 0; i < (self)->unit_num; ++i)\
    {\
        type *unit = (type*)tm_id2ptr(self, i);\
        tlibc_list_add_tail(&unit->entry.unused_list, &(self)->mempool_entry.unused_list);\
        unit->entry.sn = tm_invalid_id;\
    }\
    (self)->unused_list_num = (self)->unit_num;\
}

#define tm_empty(self) tlibc_list_empty(&(self)->mempool_entry.unused_list)
#define tm_over(self) ((self)->sn == tm_invalid_id)
#define tm_alloc(self, type, entry, unit)\
{\
    (unit) = TLIBC_CONTAINER_OF((self)->mempool_entry.unused_list.next, type, entry.unused_list);\
    tlibc_list_del(&(unit)->entry.unused_list);\
    --(self)->unused_list_num;\
    tlibc_list_add(&(unit)->entry.used_list, &(self)->mempool_entry.used_list);\
    ++(self)->used_list_num;\
    (unit)->entry.sn = (self)->sn;\
    ++(self)->sn;\
}

#define tm_free(self, type, entry, unit)\
{\
    tlibc_list_del(&(unit)->entry.used_list);\
    --(self)->used_list_num;\
    tlibc_list_add(&(unit)->entry.unused_list, &(self)->mempool_entry.unused_list);\
    ++(self)->unused_list_num;\
    (unit)->entry.sn = tm_invalid_id;\
}


extern tconnd_mempool_s g_package_pool;
extern tconnd_mempool_s g_socket_pool;

TERROR_CODE tconnd_mempool_init();

void tconnd_mempool_fini();


#endif//_H_TCONND_MEMPOOL_H

