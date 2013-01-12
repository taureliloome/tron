#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "logger.h"
#include "packets.h"
#include "socket_if.h"

int main(int argc, char *argv[])
{
	setLogLevel(LOG_LEVEL_ALL);
	uint8_t keep_alive = 1;
	uint8_t msg_type = 0, timeout = 0;
	int sockfd = 0, n = 0;
	char recvBuff[1024];

	memset(recvBuff, '0',sizeof(recvBuff));

	if(argc != 3)
	{
		printf("\n Usage: %s <ip of server> <port of server>\n",argv[0]);
		return 1;
	}

	keep_alive = ConnectToServer(argv[1], argv[2], &sockfd);

	while (keep_alive)
	{
		void *buf = RecieveMessage(sockfd, &msg_type, &timeout);
		if ( timeout == 5 ) {
			keep_alive = 0;
			ERROR("Connection to server timed out\n");
		} else {
			if ( buf ) 
			{
				switch(msg_type) {
					case 2:
						NOTICE("Connected to server, please stand by\n");
						break;
					case 3:
						DEBUG("Update information recieved\n" /*TODO add data here for debuging */);
						break;
					default:
						keep_alive = 0;
						DEBUG("Trashed unhandeled message type:%d\n", msg_type);
						break;
				}
				free(buf);
			}
			int a = 5432;
			SendMessage(sockfd, &a, sizeof(a), PCKT_EVENT);
		}
	}
	return 0;
}
