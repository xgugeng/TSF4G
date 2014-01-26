#include "tbus/tbus.h"
#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_writer.h"
#include "tcommon/tdgi_reader.h"

#include "tlibc/protocol/tlibc_binary_reader.h"
#include "tlibc/protocol/tlibc_binary_writer.h"




#include <sys/ipc.h>
#include <sys/shm.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define iSHM_KEY 10002
#define oSHM_KEY 10001


const char *message = NULL;

tbus_t *itb;
tbus_t *otb;

void block_send_pkg(tbus_t *tb, const tdgi_t *pkg)
{
    char *addr;
    tuint16 len = 0;
    TERROR_CODE ret;
    TLIBC_ERROR_CODE r;
    int idle = 0;

    TLIBC_BINARY_WRITER writer;
    len = 0;
    ret = tbus_send_begin(tb, &addr, &len);

    for(;;)
    {
        if(ret == E_TS_NOERROR)
        {
        	tlibc_binary_writer_init(&writer, addr, len);
            r = tlibc_write_tdgi_t(&writer.super, pkg);
            if(r != E_TLIBC_NOERROR)
            {
                ++len;
                ret = tbus_send_begin(tb, &addr, &len);
                continue;
            }
            else
            {
                tbus_send_end(tb, writer.offset);
                break;
            }
        }
        else if(ret == E_TS_WOULD_BLOCK)
        {
            ++idle;
            if(idle > 30)
            {
                usleep(1000);
            }
        }
        else if(ret == E_TS_AGAIN)
        {        
        }
        else
        {
            exit(1);
        }        
        len = 0;
        ret = tbus_send_begin(tb, &addr, &len);
    }
}


void process_pkg(const tdgi_t *req)
{
    tdgi_t rsp;

    switch(req->cmd)
    {
    case e_tdgi_cmd_new_connection_req:
        rsp.cmd = e_tdgi_cmd_new_connection_ack;
        rsp.mid_num = req->mid_num;
        memcpy(&rsp.mid, &req->mid, req->mid_num * sizeof(tuint64));
        block_send_pkg(otb, &rsp);
        break;
    default:
        break;
    }
}



int main()
{
    tdgi_t pkg;
	int ishm_id, oshm_id;
	tuint16 len;
	tuint16 message_len;
	TERROR_CODE ret;
	tuint32 i;
	TLIBC_BINARY_READER reader;
	TLIBC_ERROR_CODE r;
	ishm_id = shmget(iSHM_KEY, 0, 0666);
    itb = shmat(ishm_id , NULL, 0);

	
	oshm_id = shmget(oSHM_KEY, 0, 0666);
    otb = shmat(oshm_id, NULL, 0);



	for(i = 0;; ++i)
	{
		ret = tbus_read_begin(itb, &message, &message_len);
		if(ret == E_TS_NOERROR)
		{
		    len = message_len;
		    while(len > 0)
		    {
    			tlibc_binary_reader_init(&reader, message, len);
	    		r = tlibc_read_tdgi_t(&reader.super, &pkg);
	    		if(r == E_TLIBC_NOERROR)
	    		{
	    		    process_pkg(&pkg);
	    		    len -= reader.offset;
	    		}
	    		else
	    		{
	    		    printf("error\n");
	    		    exit(1);
	    		}
	    	}			
			tbus_read_end(itb, message_len);
			continue;
		}
		else if(ret == E_TS_AGAIN)
		{			
			continue;
		}
		else if(ret == E_TS_WOULD_BLOCK)
		{
			usleep(1000);
		}
		else
		{
			printf("error.\n");
			exit(1);
		}
	}
	
	return 0;
}

