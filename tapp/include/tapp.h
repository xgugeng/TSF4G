#ifndef _H_TAPP
#define _H_TAPP

#ifdef  __cplusplus
extern "C" {
#endif

#include "protocol/tlibc_xml_reader.h"
#include "tlibc_error_code.h"
#include <unistd.h>



#define TAPP_VERSION "0.0.1"
#define TAPP_IDLE_USEC 1000
#define TAPP_IDLE_LIMIT 30


//读取结构体的函数指针， 这个函数应该是由TData所生成的。
typedef tlibc_error_code_t (*tapp_xml_reader_t)(tlibc_abstract_reader_t *self, void *data);

/*
*  读取命令行参数， 如果读取失败会exit 1
*/
void tapp_load_config(void *config, int argc, char *argv[], tapp_xml_reader_t reader);


typedef TERROR_CODE (*tapp_func_t)(void *arg);
/*
*  首先会注册信号处理函数， 然后循环执行以下操作
* 1. 如果收到SIGTERM 或SIGINT 信号，主循环会break。
* 2. 如果收到SIGUSR1 信号，执行sigusr1，如果出错则返回。
* 3. 如果收到SIGUSR2 信号，执行sigusr2，如果出错则返回。
* 4. 忽略SIGPIPE信号
* 5. 执行所有的process 函数， 直到遇到一个NULL。
*      如果有process出错， 那么tapp_loop返回这个错误码
*      如果所有process都返回E_TS_WOULD_BLOCK,  那么++idle_count， 如果超过idle_limit次则usleep(usec)。
*      如果有一个process返回E_TS_NOERROR， 那么idle_count = 0。
* 例子:
*     tapp_loop(TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL, NULL, NULL
                     , process, NULL
                     , NULL, NULL)

*/
TERROR_CODE tapp_loop(useconds_t idle_usec, size_t idle_limit,
                        tapp_func_t sigusr1, void* usr1_arg,
                        tapp_func_t sigusr2, void* usr2_arg,
                        ...);




#ifdef  __cplusplus
}
#endif

#endif//_H_TAPP

