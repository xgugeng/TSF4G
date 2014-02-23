#ifndef _H_TCONND_SIGNAL_H
#define _H_TCONND_SIGNAL_H

#include "tcommon/terrno.h"

/*
DESCRIPTION
	初始化信号处理器， 注册信号SIGINT和SIGTERM的处理函数， 并初始化全局变量

RETURN VALUE
	E_TS_NOERROR:成功
	E_TS_ERROR:失败
*/
TERROR_CODE signal_processing_init();

/*
DESCRIPTION
	处理信号主循环

RETURN VALUE
	E_TS_NOERROR:成功处理一个信号
	E_TS_WOULD_BLOCK:没有需要处理的信号
*/
void signal_processing_proc();

#endif//_H_TCONND_SIGNAL_H

