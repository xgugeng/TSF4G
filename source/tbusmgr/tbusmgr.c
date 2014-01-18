#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/shm.h>



#include "tserver/tbus/tbus.h"

void version()
{
	printf ( "TBus Version %s.\n", TBUS_VERSION);
}

void help() 
{
	printf("tbusmgr ([-s size] && [-w shmkey] | --help | --version)\n");
	printf("--help\tPrint the help.\n");
	printf("--version\tPrint the version.\n");
	printf("-s size -w shmkey\tCreate shared memory segments with shmkey, and initialize the memory for tbus.\n");
}

int main(int argc, char**argv)
{
	int i;
	int shm_size = -1;
	int shm_key = 0;
	int shm_id;
	void *shm_ptr = NULL;
	tbus_t *tbus_ptr = NULL;
	int ret;

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
					fprintf(stderr, "Missing size.\n");
					help();
					exit(1);
				}
				
				errno = 0;
				shm_size = atoi(arg);
				if(errno != 0)
				{
					fprintf(stderr, "atoi(\"%s\") returned an error[%d].\n", arg, errno);
					exit(1);
				}
			}
			else if (strcmp(arg, "-w") == 0)
			{			
				arg = argv[++i];
				if(arg == NULL)
				{
					fprintf(stderr, "Missing shmkey.\n");
					help();
					exit(1);
				}

				if(shm_size < 0)
				{
					fprintf(stderr, "Please specify shared memory size use -s option.\n");
					exit(1);
				}
				errno = 0;
				shm_key = atoi(arg);
				if(errno != 0)
				{
					fprintf(stderr, "atoi(\"%s\") returned an error[%d].\n", arg, errno);
					exit(1);
				}

				errno = 0;
				shm_id = shmget(shm_key, shm_size, 0664 | IPC_CREAT|IPC_EXCL);
				if(shm_id == -1)
				{
					fprintf(stderr, "shmget(%d, %d, IPC_CREAT|IPC_EXCL) returned an error[%d].\n", shm_key, shm_size, errno);
					exit(1);
				}
				shm_ptr = shmat(shm_id, NULL, 0);
				if(shm_ptr == NULL)
				{
					fprintf(stderr, "shmat(%d, NULL, 0) returned an error[%d].\n", shm_id, errno);
					exit(1);
				}
				tbus_ptr = (tbus_t*)shm_ptr;
				ret = tbus_init(tbus_ptr, shm_size);
				if(ret != 0)
				{
					fprintf(stderr, "tbus_init(%p) returned an error[%d].\n", tbus_ptr, ret);
					exit(1);
				}
			}
			else
			{
				fprintf(stderr, "Unrecognized option: %s\n", arg);
				help();
				exit(1);
			}
			arg = strtok(NULL, " =");
		}
	}
	
	
	return 0;
}

