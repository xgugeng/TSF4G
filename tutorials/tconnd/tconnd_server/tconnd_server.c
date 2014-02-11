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

void block_send_pkg(tbus_t *tb, const tdgi_rsp_t *pkg)
{
    char *addr;
    size_t len = 0;
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
            r = tlibc_write_tdgi_rsp_t(&writer.super, pkg);
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
        else
        {
            exit(1);
        }        
        len = 0;
        ret = tbus_send_begin(tb, &addr, &len);
    }
}


tuint16 process_pkg(const tdgi_req_t *req,  const char* body_ptr)
{
    tdgi_rsp_t rsp;
    TLIBC_UNUSED(body_ptr);
    
    switch(req->cmd)
    {
    case e_tdgi_cmd_new_connection_req:
        rsp.cmd = e_tdgi_cmd_new_connection_rsp;
        rsp.mid_num = 1;
        rsp.mid[0] = req->mid;
        block_send_pkg(otb, &rsp);
        break;
    case e_tdgi_cmd_recv:
        printf("recv: %u\n", req->size);
        return req->size;
    default:
        break;
    }
    return 0;
}



int main()
{
    tdgi_req_t pkg;
	int ishm_id, oshm_id;
	tuint16 len;
	size_t message_len;
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
		        tuint16 body_size;
		        
    			tlibc_binary_reader_init(&reader, message, len);
	    		r = tlibc_read_tdgi_req_t(&reader.super, &pkg);
	    		if(r != E_TLIBC_NOERROR)
	    		{
    	    		printf("error\n");
	    		    exit(1);
	    		}		
                body_size = process_pkg(&pkg, message + pkg.size);
                len -= reader.offset + body_size;
                message += reader.offset + body_size;
	    	}			
			tbus_read_end(itb, message_len);
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

