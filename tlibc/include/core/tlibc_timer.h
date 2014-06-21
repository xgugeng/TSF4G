#ifndef _H_TLIBC_TIMER_H
#define _H_TLIBC_TIMER_H

#ifdef  __cplusplus
extern "C" {
#endif


#include "platform/tlibc_platform.h"
#include "core/tlibc_list.h"
#include "core/tlibc_error_code.h"

#include <stdint.h>
typedef struct _tlibc_timer_entry_t tlibc_timer_entry_t;
typedef void (*tlibc_timer_callback)(const struct _tlibc_timer_entry_t*);
struct _tlibc_timer_entry_t
{
	tlibc_list_head_t entry;
	uint64_t expires;
	tlibc_timer_callback callback;
};

#define TIMER_ENTRY_BUILD(en, exp, c)\
	{\
		tlibc_list_init(&(en)->entry);\
		(en)->expires = exp;\
		(en)->callback = c;\
	}

#define TLIBC_TVN_BITS (6)
#define TLIBC_TVR_BITS (8)
#define TLIBC_TVN_SIZE (1 << TLIBC_TVN_BITS)
#define TLIBC_TVR_SIZE (1 << TLIBC_TVR_BITS)
#define TLIBC_TVN_MASK (TLIBC_TVN_SIZE - 1)
#define TLIBC_TVR_MASK (TLIBC_TVR_SIZE - 1)
#define TLIBC_MAX_TVAL ((unsigned long)((1ULL << (TLIBC_TVR_BITS + 4 * TLIBC_TVN_BITS)) - 1))

typedef struct _tlibc_timver_vec_t
{
	tlibc_list_head_t vec[TLIBC_TVN_SIZE];
}tlibc_timer_vec_t;

typedef struct _tlibc_timer_verc_root_t
{
	tlibc_list_head_t vec[TLIBC_TVR_SIZE];
}tlibc_timer_ver_root_t;

typedef struct _tlibc_timer_t
{
	uint64_t jiffies;
	tlibc_timer_ver_root_t tv1;
	tlibc_timer_vec_t tv2;
	tlibc_timer_vec_t tv3;
	tlibc_timer_vec_t tv4;
	tlibc_timer_vec_t tv5;
}tlibc_timer_t;


void tlibc_timer_init(tlibc_timer_t *base);


void tlibc_timer_pop(tlibc_timer_entry_t *timer);


void tlibc_timer_push(tlibc_timer_t *self, tlibc_timer_entry_t *timer);

#define tlibc_timer_jiffies(self) ((self)->jiffies)

tlibc_error_code_t tlibc_timer_tick(tlibc_timer_t *self);


#ifdef  __cplusplus
}
#endif

#endif//_H_TLIBC_TIMER_H
