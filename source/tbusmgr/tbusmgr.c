#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#ifdef INFO_PRINT_OFF
#undef INFO_PRINT_OFF
#endif

#include "tlog/tlog_print.h"

#include <sys/ipc.h>
#include <sys/shm.h>


#include "tbus/tbus.h"

static void version()
{
	INFO_PRINT( "TBus Version %s.", TBUS_VERSION);
}

static void help() 
{
	INFO_PRINT("tbusmgr ([-s size] && [-w shmkey] | --help | --version)");
	INFO_PRINT("--help                Print the help.");
	INFO_PRINT("--version             Print the version.");
	INFO_PRINT("-s size -w shmkey     Create shared memory segments with shmkey, and initialize the memory for tbus.");
}

int main(int argc, char**argv)
{
	int i;
	size_t shm_size = 0;
	key_t shm_key = 0;
	int shm_id;
	void *shm_ptr = NULL;
	tbus_t *tbus_ptr = NULL;
	char *endptr;
	TERROR_CODE ret;

	if(argc < 2)
	{
		help();
	}
	
	for (i = 1; i < argc; ++i)
	{
		char* arg;
	
		arg = strtok(argv[i], " ");
		while (arg != NULL)
		{
			if (arg[0] == '-' && arg[1] == '-')
			{
				++arg;
		 	}
	
			if(strcmp(arg, "-help") == 0)
			{
				help();
				exit(0);
			}
			else if (strcmp(arg, "-version") == 0)
			{
				version();
				exit(0);
			}
			else if (strcmp(arg, "-s") == 0)
			{
				arg = argv[++i];
				if(arg == NULL)
				{
				    ERROR_PRINT("Missing size.");
					help();
					exit(1);
				}
				
				errno = 0;
				shm_size = (size_t)strtoull(arg, &endptr, 10);
				if(errno != 0)
				{
                    ERROR_PRINT("strtoull(\"%s\", &endptr, 10) returned an errno[%d], %s.", arg, errno , strerror(errno));
					exit(1);
				}
				if(endptr == arg)
				{
				    ERROR_PRINT("strtoull(\"%s\", &endptr, 10) return %zu, first invalid character[%c].", arg, shm_size, *endptr);
				    exit(1);
				}
			}
			else if (strcmp(arg, "-w") == 0)
			{
				arg = argv[++i];
				if(arg == NULL)
				{
					ERROR_PRINT("Missing shmkey.");
					help();
					exit(1);
				}

				errno = 0;
                shm_key = (key_t)strtol(arg, &endptr, 10);
				if(errno != 0)
				{
					ERROR_PRINT("strtol(\"%s\", &endptr, 10) returned an errno[%d], %s.", arg, errno, strerror(errno));
					exit(1);
				}
				
				if(endptr == arg)
				{
				    ERROR_PRINT("strtoull(\"%s\", &endptr, 10) return %zu, first invalid character[%c].", arg, shm_size, *endptr);
				    exit(1);
				}

				errno = 0;
				shm_id = shmget(shm_key, shm_size, 0664 | IPC_CREAT|IPC_EXCL);
				if(shm_id == -1)
				{
					ERROR_PRINT("shmget(%d, %zu, IPC_CREAT|IPC_EXCL) returned an errno[%d], %s.", shm_key, shm_size, errno, strerror(errno));
					exit(1);
				}
				shm_ptr = shmat(shm_id, NULL, 0);
				if(shm_ptr == NULL)
				{
					ERROR_PRINT("shmat(%d, NULL, 0) returned an errno[%d], %s.", shm_id, errno, strerror(errno));
					goto error_free_memory;
				}
				tbus_ptr = (tbus_t*)shm_ptr;
				ret = tbus_init(tbus_ptr, (tbus_atomic_size_t)shm_size);
				if(ret != E_TS_NOERROR)
				{
					ERROR_PRINT("tbus_init(%p) returned an error[%d].", tbus_ptr, ret);
					goto error_free_memory;
				}
				INFO_PRINT("tbus_init succeed, shm_key = [%d] shm_size = [%zu].", shm_key, shm_size);
				return 0;
			error_free_memory:
				shmctl(shm_id, IPC_RMID, 0);
				exit(1);
			}
			else
			{
				ERROR_PRINT("Unrecognized option: %s.", arg);
				help();
				exit(1);
			}
			arg = strtok(NULL, " ");
		}
	}
	
	
	return 0;
}

