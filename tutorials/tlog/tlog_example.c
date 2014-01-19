#include "tlog/tlog.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>



tlog_t g_tlog_instance;

int main()
{
	tlog_init(&g_tlog_instance, "tlog_config.xml");

	return 0;
}
