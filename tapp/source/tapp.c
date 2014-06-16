#include "tapp.h"
#include "terrno.h"
#include "string.h"
#include "protocol/tlibc_xml_reader.h"
#include "core/tlibc_string.h"

#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>


static void version()
{
	printf("TApp version %s\n", TAPP_VERSION);
}

static void usage()
{
    fprintf(stderr, "Usage: tapp_program [options] file\n");
    fprintf(stderr, "Use tapp_program --help for a list of options\n");
}

static void help()
{
	fprintf(stderr, "Usage: tapp_program [options] file\n");  
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  --version                 Print the compiler version.\n");
    fprintf(stderr, "  --help                    Print the useage.\n");
    fprintf(stderr, "  -I, --include dir         Add a directory to the list of directories\n");
    fprintf(stderr, "                            searched for include directives\n");
	fprintf(stderr, "  file                      Set the config file.\n");
}

void tapp_load_config(void *config, int argc, char *argv[], tapp_xml_reader_t reader)
{
	tlibc_xml_reader_t xml_reader;
    int opt;
    
    tlibc_xml_reader_init(&xml_reader);
    
    for(;;)
	{
		int option_index = 0;
		static struct option stlong_options[] = {
			{"help",    no_argument,        0, 'h'},
			{"version", no_argument,        0, 'v'},
			{"include", required_argument,  0, 'I'},
			{0,         0,                  0,  0 }
		};

		opt = getopt_long (argc, argv, "hvI:",
			stlong_options, &option_index);

		if (opt == -1)		
		{		    
		    break;
		}

		if((opt == ':') || (opt == '?'))
		{
		    usage();
            goto ERROR_RET;
        }

		switch( opt )
		{
		case 'h':
            help();
    		goto ERROR_RET;
    	case 'v':    	
            version();
            goto ERROR_RET;
        case 'I':
            tlibc_xml_add_include(&xml_reader, optarg);
            break;
        default:
            fprintf(stderr, "Unrecognized option: \"%s\"\n", argv[optind]);
            usage();
            goto ERROR_RET;
		}
	}

	
    if(argc - optind > 1)
    {
        fprintf(stderr, "Only one file can given as argument.\n");
        usage();
        goto ERROR_RET;
    }
    
	if(reader)
	{
    	tlibc_error_code_t r;
	    const char *config_file = NULL;
	    if(argc - optind < 1)
	    {
            fprintf(stderr, "Missing file specification\n");
            usage();
            goto ERROR_RET;
	    }
	    config_file = argv[optind];

        if(tlibc_xml_reader_push_file(&xml_reader, config_file) != E_TLIBC_NOERROR)
        {
       		fprintf(stderr, "File[%s] read aborted.\n", config_file);
            goto ERROR_RET;
        }

        r = reader(&xml_reader.super, config);
    	if(r != E_TLIBC_NOERROR)
    	{
        	const tlibc_xml_reader_yyltype_t *lo = tlibc_xml_current_location(&xml_reader);
        	if(lo)
        	{
            	fprintf(stderr, "%s(%d,%d - %d,%d) %s\n"
            	    , lo->file_name
            	    , lo->first_line, lo->first_column, lo->last_line, lo->last_column
            	    , tstrerror(r));
        	}
        	else
        	{
            	fprintf(stderr, "%s %s", config_file, tstrerror(r));
        	}   	
    		
    		tlibc_xml_reader_pop_file(&xml_reader);
    		goto ERROR_RET;
    	}
        tlibc_xml_reader_pop_file(&xml_reader);
	}
	else
	{
	    if(argc - optind > 0)
	    {
            fprintf(stderr, "The file is not being used.\n");
            usage();
            goto ERROR_RET;
	    }
	}
    
	return;
ERROR_RET:
    exit(1);
}

static bool g_sigterm = false;
static bool g_sigusr1 = false;
static bool g_sigusr2 = false;


static void on_signal(int sig)
{
    switch(sig)
    {
        case SIGINT:
        case SIGTERM:
            g_sigterm = true;
            break;
        case SIGUSR1:
            g_sigusr1 = true;
            break;
        case SIGUSR2:
            g_sigusr2 = true;
            break;
    }
}

TERROR_CODE tapp_loop(useconds_t idle_usec, size_t idle_limit,
                        tapp_func_t sigusr1, void* usr1_arg,
                        tapp_func_t sigusr2, void* usr2_arg,
                        ...)
{
	TERROR_CODE proc_ret;
    TERROR_CODE ret = E_TS_NOERROR;
	TERROR_CODE r;
    uint32_t idle_count = 0;
    struct sigaction  sa;
    va_list valist;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = on_signal;
	if(sigemptyset(&sa.sa_mask) != 0)
	{
	    ret = E_TS_ERRNO;
        goto done;
	}

	if((sigaction(SIGTERM, &sa, NULL) != 0)
	|| (sigaction(SIGINT, &sa, NULL) != 0)
	|| (sigaction(SIGUSR1, &sa, NULL) != 0)
	|| (sigaction(SIGUSR2, &sa, NULL) != 0))
	{
    	ret = E_TS_ERRNO;
        goto done;
	}

	
	sa.sa_handler = SIG_IGN;
    if(sigaction(SIGPIPE, &sa, NULL) != 0)
    {
    	ret = E_TS_ERRNO;
        goto done;
    }


    g_sigterm = false;
    for(;!g_sigterm;)
    {
        if(g_sigusr1)
        {
            g_sigusr1 = false;            
            if(sigusr1)
            {
                r = sigusr1(usr1_arg);
                if(r != E_TS_NOERROR)
                {
					ret = r;
                    goto done;
                }
            }
            idle_count = 0;
        }

        if(g_sigusr2)
        {
            g_sigusr2 = false;            
            if(sigusr2)
            {
                r = sigusr2(usr2_arg);
                if(r != E_TS_NOERROR)
                {
					ret = r;
                    goto done;
                }
            }
            idle_count = 0;
        }

        proc_ret = E_TS_WOULD_BLOCK;
        va_start(valist, usr2_arg);
        for(;;)
        {
            tapp_func_t func = NULL;
            void *arg = NULL;        

            func = va_arg(valist, tapp_func_t);
            if(func == NULL)
            {
                break;
            }            
            arg = va_arg(valist, void *);
            
            r = func(arg);
            if(r == E_TS_NOERROR)
            {
                proc_ret = E_TS_NOERROR;
            }
            else if(r != E_TS_WOULD_BLOCK)
            {
                proc_ret = r;
                break;
            }
        }
        va_end(valist);

        switch(proc_ret)
        {
        case E_TS_NOERROR:
            idle_count = 0;
            break;
        case E_TS_WOULD_BLOCK:
            {
                ++idle_count;
                if(idle_count >= idle_limit)
                {
                    if((usleep(idle_usec) != 0) && (errno != EINTR))
                    {
                        ret = E_TS_ERRNO;
                        goto done;
                    }
                    idle_count = 0;                 
                }
            }
            break;
        default:
			ret = proc_ret;
            goto done;
        }
    }   

done:
    return ret;
}

