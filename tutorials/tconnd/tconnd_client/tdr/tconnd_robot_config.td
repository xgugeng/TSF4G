import "tcommon.td"

struct tconnd_robot_config
{
	uint16								robot_num;
	string<TSERVER_IP_LENGTH> 			ip;
	uint16 								port;
	uint32								speed;//byte/s
};

