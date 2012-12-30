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
    int sockfd = 0, n = 0;
    char recvBuff[1024];

    memset(recvBuff, '0',sizeof(recvBuff));

    if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> <port of server>\n",argv[0]);
        return 1;
    }

    ConnectToServer(argv[1], argv[2], &sockfd);

    while (1)
    {
		int *num = RecieveMessage(sockfd);
		if ( num ) 
		{
			printf("recieved: %d\n", *num);
			free(num);
			break;
		}
    }

    return 0;
}
