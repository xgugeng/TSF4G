#include <stdio.h>
#include <string.h>

#include "terrno.h"
#include "tlog_print.h"
#include "tlogd_config_reader.h"
#include "protocol/tlibc_xml_reader.h"

#define TLOGD_VERSION "0.0.1"

tlogd_config_t g_config;

static void version()
{
	printf("tlogd version %s\n", TLOGD_VERSION);
}

static void help()
{
	fprintf(stderr, "Usage: tlogd [options] file\n");  
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -version                 Print the compiler version.\n");
    fprintf(stderr, "  -help                    Print the useage.\n");
	fprintf(stderr, "  file                     Set the config file.\n");
}

static TERROR_CODE tlogd_config_init(const char* config_file)
{
	TLIBC_XML_READER xml_reader;


    tlibc_xml_reader_init(&xml_reader);
    if(tlibc_xml_reader_push_file(&xml_reader, config_file) != E_TLIBC_NOERROR)
    {
   		ERROR_PRINT("load push config file [%s] failed.", config_file);
        goto ERROR_RET;
    }
    
	if(tlibc_read_tlogd_config_t(&xml_reader.super, &g_config) != E_TLIBC_NOERROR)
	{
    	const TLIBC_XML_READER_YYLTYPE *lo = tlibc_xml_current_location(&xml_reader);
    	if(lo)
    	{	
        	ERROR_PRINT("load read file [%s] failed at %d,%d - %d,%d.", lo->file_name, 
        	    lo->first_line, lo->first_column, lo->last_line, lo->last_column);
    	}
    	else
    	{
    	ERROR_PRINT("load read file [%s] failed.", config_file);
    	}   	
		
		tlibc_xml_reader_pop_file(&xml_reader);
		goto ERROR_RET;
	}
    tlibc_xml_reader_pop_file(&xml_reader);
    INFO_PRINT("tconnd_config_init(%s) succeed.", config_file);
    
	return E_TS_NOERROR;
ERROR_RET:
	return E_TS_ERROR;
}

int main(int argc, char **argv)
{
	int i;
	const char* config_file = NULL;

	
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

	if(tlogd_config_init(config_file))
	{
	}


	return 0;
ERROR_RET:
	return 1;
}

