#ifndef _H_TLIBC_MEMPOOL_H
#define _H_TLIBC_MEMPOOL_H

#ifdef  __cplusplus
extern "C" {
#endif


#include "platform/tlibc_platform.h"
#include "core/tlibc_list.h"
#include "tlibc_error_code.h"
typedef struct _tlibc_mempool_entry_t
{
	uint64_t          sn;
	tlibc_list_head_t  unused_list;
	tlibc_list_head_t  used_list;
}tlibc_mempool_entry_t;

typedef struct _tlibc_mempool_t
{
	char                        *pool;
	uint64_t                     sn;
	size_t                      unit_size;
	size_t                      unit_num;
	tlibc_mempool_entry_t       mempool_entry;
	size_t                      unused_list_num;
	size_t                      used_list_num;
}tlibc_mempool_t;

#define tlibc_mempool_invalid_id UINT64_MAX


#define tlibc_mempool_id_test(self, id) (id < (self)->unit_num)
#define tlibc_mempool_id2ptr(self, id) ((self)->pool + (self)->unit_size * id)
#define tlibc_mempool_ptr_test(ptr, entry, sno) (((ptr)->entry.sn != tlibc_mempool_invalid_id) && ((ptr)->entry.sn == sno))

#define tlibc_mempool_ptr2id(self, ptr) ((size_t)((char*)ptr - (self)->pool) / (self)->unit_size)

#define tlibc_mempool_init(self, type, entry, p, us, un)\
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
	type *unit = (type*)tlibc_mempool_id2ptr(self, i);\
	tlibc_list_add_tail(&unit->entry.unused_list, &(self)->mempool_entry.unused_list);\
	unit->entry.sn = tlibc_mempool_invalid_id;\
	}\
	(self)->unused_list_num = (self)->unit_num;\
}

#define tlibc_mempool_empty(self) tlibc_list_empty(&(self)->mempool_entry.unused_list)
#define tlibc_mempool_over(self) ((self)->sn == tlibc_mempool_invalid_id)
#define tlibc_mempool_alloc(self, type, entry, unit)\
{\
	(unit) = TLIBC_CONTAINER_OF((self)->mempool_entry.unused_list.next, type, entry.unused_list);\
	tlibc_list_del(&(unit)->entry.unused_list);\
	--(self)->unused_list_num;\
	tlibc_list_add(&(unit)->entry.used_list, &(self)->mempool_entry.used_list);\
	++(self)->used_list_num;\
	(unit)->entry.sn = (self)->sn;\
	++(self)->sn;\
}

#define tlibc_mempool_free(self, type, entry, unit)\
{\
	tlibc_list_del(&(unit)->entry.used_list);\
	--(self)->used_list_num;\
	tlibc_list_add(&(unit)->entry.unused_list, &(self)->mempool_entry.unused_list);\
	++(self)->unused_list_num;\
	(unit)->entry.sn = tlibc_mempool_invalid_id;\
}

#ifdef  __cplusplus
}
#endif


#endif//_H_TLIBC_MEMPOOL_H
