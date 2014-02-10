#include "globals.h"

#include "tlibc/protocol/tlibc_xml_reader.h"
#include "tconnd/tconnd_config_reader.h"
#include "tdtp_instance.h"

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
	
    tlibc_xml_reader_init(&xml_reader);
    if(tlibc_xml_reader_push_file(&xml_reader, config_file) != E_TLIBC_NOERROR)
    {
   		fprintf(stderr, "load push config file [%s] failed.\n", config_file);
        goto ERROR_RET;
    }
    
	if(tlibc_read_tconnd_config_t(&xml_reader.super, &g_config) != E_TLIBC_NOERROR)
	{
		fprintf(stderr, "load read file [%s] failed.\n", config_file);
		tlibc_xml_reader_pop_file(&xml_reader);
		goto ERROR_RET;
	}
    tlibc_xml_reader_pop_file(&xml_reader);

	ret = tdtp_instance_init(&g_tdtp_instance);
	if(ret != E_TS_NOERROR)
	{
		goto ERROR_RET;
	}
	
	for(;;)
	{
		ret = tdtp_instance_process(&g_tdtp_instance);
		if(ret == E_TS_AGAIN)
		{
			++idle_count;
			if(idle_count > 30)
			{
				usleep(1000);
				idle_count = 0;
			}
			else
			{
				sched_yield();
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

