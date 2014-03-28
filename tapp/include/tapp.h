#ifndef _H_TAPP
#define _H_TAPP

#include "protocol/tlibc_xml_reader.h"
#include "terrno.h"
#include <unistd.h>



#define TAPP_VERSION "0.0.1"

//读取结构体的函数指针， 这个函数应该是由TData所生成的。
typedef TLIBC_ERROR_CODE (*tapp_xml_reader_t)(TLIBC_ABSTRACT_READER *self, void *data);

/*
*  读取命令行参数， 如果读取失败会exit 1
*/
void tapp_load_config(void *config, int argc, char *argv[], tapp_xml_reader_t reader);

/*
* 主循环函数
* E_TS_NOERROR 成功处理一个任务
* E_TS_WOULD_BLOCK 没有需要处理的任务
* 其他返回值为出错情况
*/
typedef TERROR_CODE (*tapp_process_t)();

/*
*  首先会注册信号处理函数
* SIGTERM 函数退出
*
*然后会循环执行process
*  如果process 返回E_TS_NOERROR则会马上进入下一次process
*  如果process 连续返回E_TS_WOULD_BLOCK 超过idle_limit次, 则usleep(usec)。
*  如果process 出错， 那么tapp_loop函数会返回process的这个错误码。
*/
TERROR_CODE tapp_loop(tapp_process_t process, useconds_t usec, size_t idle_limit);



#endif//_H_TAPP
