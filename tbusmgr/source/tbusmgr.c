#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <getopt.h>

#ifdef INFO_PRINT_OFF
#undef INFO_PRINT_OFF
#endif

#include "tlog_print.h"

#include <sys/shm.h>


#include "tbus.h"

static void version()
{
	INFO_PRINT( "TBus Version %s.", TBUS_VERSION);
}

static void help() 
{
	INFO_PRINT("tbusmgr ([-s size] && [-n number] && [-w shmkey] | --help | --version)");
	INFO_PRINT("--help, -v                Print the help.");
	INFO_PRINT("--version, -h             Print the version.");
	INFO_PRINT("--size, -s size           Set the maximum packet size.");
	INFO_PRINT("--number, -n number       Set the maxinum packet number.");
	INFO_PRINT("--write, -w tbuskey       Create shared memory segments with shmkey, and initialize the memory for tbus.");
}

const char* const short_options = "vhs:n:w:";

const struct option long_options[] = 
{ 
	{ "version",   0, NULL, 'v' },
	{ "help"   ,   0, NULL, 'h' },
	{ "size"   ,   1, NULL, 's' },
	{ "number" ,   1, NULL, 'n' },
	{ "write"  ,   1, NULL, 'w' },
	{ NULL     ,   0, NULL,  0  },  
};

int main(int argc, char**argv)
{
	int opt;
	const char *arg = NULL;
	size_t size = 0;
	size_t number = 0;

	size_t shm_size = 0;
	key_t shm_key = 0;
	int shm_id;
	void *shm_ptr = NULL;
	tbus_t *tbus_ptr = NULL;
	char *endptr;

	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1)
	{
		switch(opt)
		{
		case 'h':
			help();
			exit(0);
		case 'v':
			version();
			exit(0);
		case 's':
			{
				arg = optarg;
				errno = 0;
				size = (size_t)strtoull(arg, &endptr, 10);
				if(errno != 0)
				{
					ERROR_PRINT("strtoull(\"%s\", &endptr, 10) returned an errno[%d], %s.", arg, errno , strerror(errno));
					exit(1);
				}
				if(endptr == arg)
				{
					ERROR_PRINT("strtoull(\"%s\", &endptr, 10) return %zu, first invalid character[%c].", arg, size, *endptr);
					exit(1);
				}
				break;
			}
		case 'n':
			{
				arg = optarg;
				errno = 0;
				number = (size_t)strtoull(arg, &endptr, 10);
				if(errno != 0)
				{
					ERROR_PRINT("strtoull(\"%s\", &endptr, 10) returned an errno[%d], %s.", arg, errno , strerror(errno));
					exit(1);
				}
				if(endptr == arg)
				{
					ERROR_PRINT("strtoull(\"%s\", &endptr, 10) return %zu, first invalid character[%c].", arg, number, *endptr);
					exit(1);
				}
				break;
			}
		case 'w':
			{
				arg = optarg;

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
				shm_size = TLIBC_OFFSET_OF(tbus_t, buff) + (size + sizeof(tbus_header_t)) * number;

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
				tbus_init(tbus_ptr, size, number);				
				INFO_PRINT("tbus_init succeed, shm_key = [%d] shm_size = [%zu].", shm_key, shm_size);
				return 0;
			error_free_memory:
				shmctl(shm_id, IPC_RMID, 0);
				exit(1);
			}
		}
	}

	return 0;
}

