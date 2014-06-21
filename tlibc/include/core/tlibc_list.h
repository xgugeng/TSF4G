#ifndef _H_TLIBC_LIST
#define _H_TLIBC_LIST

#ifdef  __cplusplus
extern "C" {
#endif


typedef struct tlibc_list_head_s
{
	struct tlibc_list_head_s *next, *prev;
}tlibc_list_head_t;

#define tlibc_list_init(_head)\
{\
	(_head)->next = _head;\
	(_head)->prev = _head;\
}

#define	tlibc_list_empty(list) ((list)->next == list)

#define __tlibc_list_add(_new, _prev, _next)\
{\
	(_new)->next = (_next);\
	(_new)->prev = (_prev);\
	(_new)->next->prev = (_new);\
	(_new)->prev->next = (_new);\
}

#define tlibc_list_add(_new, _head)\
{\
	__tlibc_list_add((_new), (_head), ((_head)->next));\
}

#define tlibc_list_add_tail(_new, _head)\
{\
	__tlibc_list_add((_new), (_head)->prev, (_head));\
}

#define __tlibc_list_del(_prev, _next)\
{\
	(_next)->prev = (_prev);\
	(_prev)->next = (_next);\
}

#define tlibc_list_del(entry)\
{\
	__tlibc_list_del((entry)->prev, (entry)->next);\
}

#ifdef  __cplusplus
}
#endif


#endif
