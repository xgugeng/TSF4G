#ifndef _H_ROBOT_PROTO_H
#define _H_ROBOT_PROTO_H

#define ROBOT_STR_LEN 126
typedef enum robot_proto_message_id_e
{
	e_robot_login_req = 0,
	e_robot_login_rsp = 1,
}robot_proto_message_id_t;

typedef struct robot_proto_login_req_s
{
	char name[ROBOT_STR_LEN];
	char pass[ROBOT_STR_LEN];
}robot_proto_login_req_t;

typedef struct robot_proto_login_rsp_s
{
	char name[ROBOT_STR_LEN];
	uint32_t sid;
}robot_proto_login_rsp_t;

typedef union robot_proto_body_u
{
	robot_proto_login_req_t login_req;
	robot_proto_login_rsp_t login_rsp;
}robot_proto_body_t;

typedef struct robot_proto_s
{
	robot_proto_message_id_t message_id;
	robot_proto_body_t message_body;
}robot_proto_t;

#endif
