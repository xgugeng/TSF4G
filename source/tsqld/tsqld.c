#include <stdio.h>
#include <string.h>
#include "tlog/tlog_print.h"
#include "tlog/tlog_log.h"
#include "tlibc/protocol/tlibc_xml_reader.h"
#include "tsqld/tsqld_config_types.h"
#include "tsqld/tsqld_config_reader.h"

#define PROGRAN_NAME "tsqld"
#define TSQLD_VERSION "0.0.1"

tsqld_config_t g_config;

static void version()
{
	INFO_PRINT("%s version %s.", PROGRAN_NAME, TSQLD_VERSION);
}

static void help()
{
	INFO_PRINT("Usage: %s [options] file.", PROGRAN_NAME);  
	INFO_PRINT("Options:");
	INFO_PRINT("  -version                 Print the compiler version.");
    INFO_PRINT("  -help                    Print the useage.");
    INFO_PRINT("  -i                       Add include path.");
    INFO_PRINT("  -l                       Set the log config file.");
	INFO_PRINT("  file                     Set the config file.");
}

static void load(int argc, char* argv[])
{
	int i;
	const char* config_file = NULL;
    TLIBC_XML_READER xml_reader;
    tlibc_xml_reader_init(&xml_reader);
    TLIBC_ERROR_CODE r;

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
			exit(0);
		}
		else if (strcmp(arg, "-version") == 0)
		{
			version();
			exit(0);
		}
		else if (strcmp(arg, "-l") == 0)
		{
		    arg = argv[++i];
 
            if(tlog_init(&g_tlog_instance, arg) != E_TS_NOERROR)
            {
                ERROR_PRINT("tlog init [%s] failed.", arg);
                exit(1);
            }
            INFO_PRINT("tlog init(%s) succeed, check the log file for more information.", arg);
            
			exit(0);
		}
		else if (strcmp(arg, "-i") == 0)
		{
		    arg = argv[++i];

			if(tlibc_xml_add_include(&xml_reader, arg) != E_TLIBC_NOERROR)
			{
				ERROR_PRINT("Too many include path [%s].", arg);
				exit(1);
			}
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
		ERROR_PRINT("Missing config file specification");
		help();
		exit(1);
	}

    if(tlibc_xml_reader_push_file(&xml_reader, config_file) != E_TLIBC_NOERROR)
    {   
        ERROR_PRINT("load push config file [%s] failed.", config_file);
        exit(1);
    }

    r = tlibc_read_tsqld_config_t(&xml_reader.super, &g_config);
	if(r != E_TLIBC_NOERROR)
    {   
        const TLIBC_XML_READER_YYLTYPE *lo = tlibc_xml_current_location(&xml_reader);
        if(lo)
        {   
            ERROR_PRINT("load xml [%s] return [%d] at %d,%d - %d,%d.", lo->file_name, r,
                lo->first_line, lo->first_column, lo->last_line, lo->last_column);
        }   
        else
        {   
	        ERROR_PRINT("load xml [%s] return [%d].", config_file, r);
        }

        tlibc_xml_reader_pop_file(&xml_reader);
		exit(1);
    }
    tlibc_xml_reader_pop_file(&xml_reader);
}

int main(int argc, char *argv[])
{
	load(argc, argv);

	return 0;
}

