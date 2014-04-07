#ifndef _H_TAPP
#define _H_TAPP

#include "protocol/tlibc_xml_reader.h"
#include "terrno.h"
#include <unistd.h>



#define TAPP_VERSION "0.0.1"
#define TAPP_IDLE_USEC 1000
#define TAPP_IDLE_LIMIT 30


//读取结构体的函数指针， 这个函数应该是由TData所生成的。
typedef TLIBC_ERROR_CODE (*tapp_xml_reader_t)(TLIBC_ABSTRACT_READER *self, void *data);

/*
*  读取命令行参数， 如果读取失败会exit 1
*/
void tapp_load_config(void *config, int argc, char *argv[], tapp_xml_reader_t reader);


typedef TERROR_CODE (*tapp_func_t)();
/*
*  首先会注册信号处理函数， 然后循环执行以下操作
* 1. 如果收到SIGTERM 或SIGINT 信号，主循环会break。
* 2. 如果收到SIGUSR1 信号，执行sigusr1，如果出错则返回。
* 3. 如果收到SIGUSR2 信号，执行sigusr2，如果出错则返回。
* 4. 忽略SIGPIPE信号
* 5. 执行process
*      如果process 返回E_TS_NOERROR则会马上进入下一次process
*      如果process 连续返回E_TS_WOULD_BLOCK 超过idle_limit次, 则usleep(usec)。
*      如果process 出错， 那么tapp_loop函数会返回process的这个错误码。
*/
TERROR_CODE tapp_loop(tapp_func_t process, useconds_t idle_usec, size_t idle_limit,
                        tapp_func_t sigusr1, tapp_func_t sigusr2);



#endif//_H_TAPP
