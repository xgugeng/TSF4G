#include <stdio.h>

#include "tserver/tbus/tbus.h"

void useage() 
{
	printf ( "\n" ) ;
	printf ( "TBus Version %s.\n", TBUS_VERSION);
	printf ( "Usage:\n" ) ;
	printf ( "tbusmgr ([-s size] && [-w id] | [-d id])\n");
	printf ( "-w size -w id\t创建名叫id， 大小为size的共享内存\n");
	printf ( "-d id\t删除id通道\n");
}

int main()
{

	useage();
	
	test();
	
	return 0;
}

