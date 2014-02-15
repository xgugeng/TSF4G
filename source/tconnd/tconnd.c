#include "globals.h"
#include "tdtp_instance.h"
#include <stdio.h>
#include <string.h>

void version()
{
	printf("TConnd version %s\n", TCONND_VERSION);
}

void help()
{
	fprintf(stderr, "Usage: tconnd [options] file\n");  
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -version                 Print the compiler version.\n");
    fprintf(stderr, "  -help                    Print the useage.\n");
	fprintf(stderr, "  file                     Set the config file.\n");
}

int main(int argc, char **argv)
{
	int i, ret;

	
	for (i = 1; i < argc; ++i)
	{
		char* arg;

		arg = strtok(argv[i], " ");
		if (arg[0] == '-' && arg[1] == '-')
		{
			++arg;
		}
		if (strcmp(arg, "-help") == 0)
		{
			help();
			goto ERROR_RET;
		}
		else if (strcmp(arg, "-version") == 0)
		{
			version();
			goto ERROR_RET;
		}
		else
		{
		    g_config_file = arg;
            break;
		}

		arg = strtok(NULL, " =");
	}
	if (g_config_file == NULL)
	{
		fprintf(stderr, "Missing config file specification\n");
		help();
		goto ERROR_RET;
	}


	ret = tdtp_instance_init(&g_tdtp_instance);
	if(ret != E_TS_NOERROR)
	{
		goto ERROR_RET;
	}   

	tdtp_instance_loop(&g_tdtp_instance);

	tdtp_instance_fini(&g_tdtp_instance);

	return 0;
ERROR_RET:
	return 1;
}

