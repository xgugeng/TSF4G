#include "globals.h"

#include "tlibc/protocol/tlibc_xml_reader.h"
#include "tconnd/tconnd_config_reader.h"
#include "tdtp_instance.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_writer.h"
#include "tlibc/protocol/tlibc_binary_writer.h"



void version()
{
	printf("TConnd version %s\n", TCONND_VERSION);
}

void help()
{
	fprintf(stderr, "Usage: tconnd [options] file\n");  
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -version					Print the compiler version.\n");
    fprintf(stderr, "  -help					Print the useage.\n");
	fprintf(stderr, "  file					    Set the config file.\n");
}

int main(int argc, char **argv)
{
	TLIBC_XML_READER xml_reader;
	const char *config_file = NULL;
	int i, ret;
    tdgi_req_t pkg;
    TLIBC_BINARY_WRITER writer;
    char pkg_buff[sizeof(tdgi_req_t)];
	
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
		    config_file = arg;
            break;
		}

		arg = strtok(NULL, " =");
	}
	if (config_file == NULL)
	{
		fprintf(stderr, "Missing config file specification\n");
		help();
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



    tlibc_binary_writer_init(&writer, pkg_buff, sizeof(pkg_buff));
    memset(&pkg, 0, sizeof(pkg));
    if(tlibc_write_tdgi_req_t(&writer.super, &pkg) != E_TLIBC_NOERROR)
    {
        goto ERROR_RET;
    }
    g_head_size = writer.offset;

 
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

