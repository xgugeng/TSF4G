#include "globals.h"

#include "tlibc/protocol/tlibc_xml_reader.h"
#include "tconnd/tconnd_config_reader.h"
#include "tdtp/tdtp_instance.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>


void version()
{
	printf("TConnd version %s\n", TCONND_VERSION);
}

void usage()
{
	fprintf(stderr, "Usage: tconnd [options] file\n\n");
	fprintf(stderr, "Use tdata -help for a list of options\n");
}

void help()
{
	fprintf(stderr, "Usage: tconnd [options] file\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -version					Print the compiler version\n");
	fprintf(stderr, "  -c file					Set the config file\n");
}



TERROR_CODE instance_init()
{
	tuint32 i;
	TERROR_CODE ret = E_TS_NOERROR;

	for(i = 0; i < g_config.tdtp_num; ++i)
	{
		ret = tdtp_instance_init(&g_tdtp_instance[i], &g_config.tdtp[i]);
		if(ret != E_TS_NOERROR)
		{
			goto done;
		}
	}

done:
	return ret;
}

TERROR_CODE instance_process()
{
	tuint32 i;
	TERROR_CODE ret = E_TS_AGAIN;

	for(i = 0; i < g_config.tdtp_num; ++i)
	{
		TERROR_CODE r = tdtp_instance_process(&g_tdtp_instance[i]);
		if(r == E_TS_NOERROR)
		{
			ret = E_TS_NOERROR;
		}
		else if(r != E_TS_AGAIN)
		{
			ret = r;
			goto done;
		}
		
	}
done:
	return ret;
}

int main(int argc, char **argv)
{
	TLIBC_XML_READER xml_reader;
	const char *config_file = NULL;
	int i, ret;
	tuint32 idle_count = 0;
	
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
		else if (strcmp(arg, "-c") == 0)
		{			
			config_file = argv[++i];
		}
		else
		{
	        fprintf(stderr, "Unrecognized option: %s\n", arg);
            help();
			goto ERROR_RET;
		}

		arg = strtok(NULL, " =");
	}
	if (config_file == NULL)
	{
			fprintf(stderr, "Missing config file specification\n");
			usage();
			goto ERROR_RET;
	}

	if((tlibc_xml_reader_init(&xml_reader, config_file) != E_TLIBC_NOERROR)
		||(tlibc_read_tconnd_config_t(&xml_reader.super, &g_config) != E_TLIBC_NOERROR))
	{
		fprintf(stderr, "load config file [%s] failed.\n", config_file);
		goto ERROR_RET;
	}

	ret = instance_init();
	if(ret != E_TS_NOERROR)
	{
		goto ERROR_RET;
	}
	
	for(;;)
	{
		ret = instance_process();
		if(ret == E_TS_AGAIN)
		{
			++idle_count;
			if(idle_count > 30)
			{
//				usleep(1000);
				idle_count = 0;
			}
			else
			{
//				sched_yield();
			}
		}
		else if(ret != E_TS_NOERROR)
		{
			goto ERROR_RET;
		}
	}
	
	return 0;
ERROR_RET:
	return 1;
}

