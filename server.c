#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "packets.h"
#include "socket_if.h"

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;

	if ( argc != 2 )
	{
        printf("\n Usage: %s <port of server>\n",argv[0]);
		return 1;
	}
    CreateListenSocket(argv[1], &listenfd);

    while(1)
    {
		connfd = ServerAcceptClient(&listenfd);
		int a = 5639;
		SendMessage(connfd, &a, sizeof(a), PCKT_UPDATE);

        close(connfd);
        break;
	}
	close(listenfd);
	return 0;
}
