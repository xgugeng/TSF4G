#include "tapp.h"
#include "tbus.h"
#include "sip.h"
#include "tconnd_proto.h"

#include "tlog_log.h"
#ifdef MAKE_RELEASE
#define DEBUG_PRINT_OFF
#define INFO_PRINT_OFF
#endif
#include "tlog_print.h"
#include "bscp_types.h"
#include "tconnapi.h"


#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>


#define iSHM_KEY 10002
#define oSHM_KEY 10001

tconnapi_t g_tconn;


static sip_size_t robot_proto_encode(const robot_proto_t *self, char *start, char *limit)
{
	if(limit - start < sizeof(robot_proto_t))
	{
		return 0;
	}

	memcpy(start, self, sizeof(robot_proto_t));

	return sizeof(robot_proto_t);
}


static void on_connect(tconnapi_t *self, const sip_cid_t *cid)
{
	tconnapi_accept(self, cid, 1);
    INFO_PRINT("[%u, %llu] accept.", cid->id, cid->sn);
}

static void on_close(tconnapi_t *self, const sip_cid_t *cid)
{
	INFO_PRINT("[%u, %llu] client close.", cid->id, cid->sn);
}

static void on_recv(tconnapi_t *self, const sip_cid_t *cid, const char *packet, sip_size_t packet_size)
{
	const robot_proto_t *msg_req = (const robot_proto_t*)packet;
	robot_proto_t msg_rsp;

	if(packet_size != sizeof(robot_proto_t))
	{
		ERROR_PRINT("protocol decode failed.");
		tconnapi_close(self, cid, 1);
		goto done;
	}

	msg_rsp.message_id = e_robot_login_rsp;
	memcpy(&msg_rsp.message_body.login_rsp.name, &msg_req->message_body.login_req.name, ROBOT_STR_LEN);
	msg_rsp.message_body.login_rsp.sid = (uint32_t)atoi(msg_req->message_body.login_req.pass);

	tconnapi_send(self, cid, 1, &msg_rsp);
done:
	return;
}

int main()
{
	if(tapp_sigaction() != E_TLIBC_NOERROR)
	{
		ERROR_PRINT("tapp_sigaction failed.");
		return 1;
	}

	if(tconnapi_init(&g_tconn, iSHM_KEY, oSHM_KEY, (encode_t)robot_proto_encode) != E_TLIBC_NOERROR)
	{
		ERROR_PRINT("tconnapi_init failed.");
		return 1;
	}

	g_tconn.on_connect = on_connect;
	g_tconn.on_recv = on_recv;
	g_tconn.on_close = on_close;

    return tapp_loop(TAPP_IDLE_USEC, TAPP_IDLE_LIMIT
                     , tconnapi_process, &g_tconn
                     , NULL, NULL);
}

