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
#include "packets.h"
#include "socket_if.h"

int main(int argc, char *argv[])
{
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

	ConnectToServer(argv[1], argv[2], &sockfd);

	while (keep_alive)
	{
		void *buf = RecieveMessage(sockfd, &msg_type, &timeout);
		if ( timeout == 5 ) {
			keep_alive = 0;
			printf("Connection to server timed out\n");
		} else {
			if ( buf ) 
			{
				switch(msg_type) {
					case 2:
						printf("connection accepted - execute connection sequences!!!!!!\n");
						break;
					case 3:
						//printf("new intelengence has been recieved, update our databases\n");
						break;
					default:
						keep_alive = 0;
						printf("we are burried in trash T_T\n");
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
