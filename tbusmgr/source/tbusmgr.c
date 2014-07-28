#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <getopt.h>
#include <sys/shm.h>

#include "tlibcdef.h"
#include "tbus.h"

static void version()
{
	fprintf(stdout, "TBus Version %s.\n", TBUS_VERSION);
}

static void help() 
{
	fprintf(stdout, "tbusmgr ([-s size] && [-n number] && [-w shmkey] | --help | --version)\n");
	fprintf(stdout, "--help, -h                Print the help.\n");
	fprintf(stdout, "--version, -v             Print the version.\n");
	fprintf(stdout, "--size, -s size           Set the maximum packet size.\n");
	fprintf(stdout, "--number, -n number       Set the maxinum packet number.\n");
	fprintf(stdout, "--write, -w tbuskey       Create shared memory segments with shmkey, and initialize the memory for tbus.\n");
	fprintf(stdout, "--dashboard, -d tbuskey   Show dashboard of the tbus channel.\n");
}

const char* const short_options = "vhs:n:w:";

const struct option long_options[] = 
{ 
	{ "version",   0, NULL, 'v' },
	{ "help"   ,   0, NULL, 'h' },
	{ "size"   ,   1, NULL, 's' },
	{ "number" ,   1, NULL, 'n' },
	{ "write"  ,   1, NULL, 'w' },
	{ "dashboard", 1, NULL, 'd' },
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
	int shm_id = 0;
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
		case 'd':
			{
				tbus_t *tb = NULL;
				arg = optarg;
				uint32_t head, tail, size;


				errno = 0;
				shm_key = (key_t)strtol(arg, &endptr, 10);
				if(errno != 0)
				{
					fprintf(stderr, "strtol(\"%s\", &endptr, 10) returned an errno[%d], %s.\n", arg, errno, strerror(errno));
					exit(1);
				}

				tb = tbus_at(shm_key);
				if(tb == NULL)
				{
					fprintf(stderr, "tbus_at(%d) failed, errno(%d), %s.\n", shm_key, errno, strerror(errno));
					goto error_free_memory;
				}
				head = tb->head_offset;
				tail = tb->tail_offset;
				if(head <= tail)
				{
					size = tail - head;
				}
				else
				{
					size = head - tail;
					size = tb->size - size;
				}

				fprintf(stdout, "used : %u, total : %u, percent : %.2lf%%.\n", size, tb->size, (double)size / (double)tb->size);
				exit(0);
			}
		case 's':
			{
				arg = optarg;
				errno = 0;
				size = (size_t)strtoull(arg, &endptr, 10);
				if(errno != 0)
				{
					fprintf(stderr, "strtoull(\"%s\", &endptr, 10) returned an errno[%d], %s.\n", arg, errno , strerror(errno));
					exit(1);
				}
				if(endptr == arg)
				{
					fprintf(stderr, "strtoull(\"%s\", &endptr, 10) return %zu, first invalid character[%c].\n", arg, size, *endptr);
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
					fprintf(stderr, "strtoull(\"%s\", &endptr, 10) returned an errno[%d], %s.\n", arg, errno , strerror(errno));
					exit(1);
				}
				if(endptr == arg)
				{
					fprintf(stderr, "strtoull(\"%s\", &endptr, 10) return %zu, first invalid character[%c].\n", arg, number, *endptr);
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
					fprintf(stderr, "strtol(\"%s\", &endptr, 10) returned an errno[%d], %s.\n", arg, errno, strerror(errno));
					exit(1);
				}

				if(endptr == arg)
				{
					fprintf(stderr, "strtoull(\"%s\", &endptr, 10) return %zu, first invalid character[%c].\n", arg, shm_size, *endptr);
					exit(1);
				}
				shm_size = tbus_size(size, number);

				errno = 0;
				shm_id = shmget(shm_key, shm_size, 0664 | IPC_CREAT|IPC_EXCL);
				if(shm_id == -1)
				{
					fprintf(stderr, "shmget(%d, %zu, IPC_CREAT|IPC_EXCL) returned an errno[%d], %s.\n", shm_key, shm_size, errno, strerror(errno));
					exit(1);
				}
				shm_ptr = shmat(shm_id, NULL, 0);
				if(shm_ptr == NULL)
				{
					fprintf(stderr, "shmat(%d, NULL, 0) returned an errno[%d], %s.\n", shm_id, errno, strerror(errno));
					goto error_free_memory;
				}
				tbus_ptr = (tbus_t*)shm_ptr;
				tbus_init(tbus_ptr, size, number);				
				fprintf(stdout, "tbus_init succeed, shm_key = [%d] shm_size = [%zu].\n", shm_key, shm_size);
				return 0;
			error_free_memory:
				shmctl(shm_id, IPC_RMID, 0);
				exit(1);
			}
        default:
            {
                fprintf(stderr, "Unrecognized option: \"%s\"\n", argv[optind]);
                help();
                exit(1);
            }
		}
	}

    fprintf(stderr, "Missing --write\n");
    help();
	return 1;
}

