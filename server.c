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
#include <pthread.h>

#include "logger.h"
#include "packets.h"
#include "socket_if.h"

static void* clientHandler(void *fd);
static void waitForPlayers(int reqClientCount, int listenfd);

pthread_t accept_thread;
pthread_t derive_thread;

typedef enum {
	GS_STARTUP,
	GS_IN_PROGRESS,
	GS_ENDING,
	GS_MAX
} game_state_t;

static uint8_t game_state = GS_STARTUP;

int main(int argc, char *argv[])
{
	setLogLevel(LOG_LEVEL_ALL);
    int listenfd = 0, playerCount = 5;

	if ( argc != 2 )
	{
        printf("\n Usage: %s <port of server>\n",argv[0]);
		return 1;
	}
    CreateListenSocket(argv[1], &listenfd);
	
	waitForPlayers(playerCount, listenfd);

	close(listenfd);
	return 0;
}

static void waitForPlayers(int reqClientCount, int listenfd)
{
	int connfd = 0;
    while(getClientCount() < reqClientCount)
    {
		connfd = ServerAcceptClient(&listenfd);

		if ( connfd ) {
			NOTICE("<SERVER> New player connected\n");
			uint8_t client_count = getClientCount();
			pthread_create(&derive_thread, 0, &clientHandler, (void*)&connfd );
			pthread_detach(derive_thread);
		}
	}
	NOTICE("<SERVER> All players connected\n");
	game_state = GS_IN_PROGRESS;
}

static void* clientHandler(void *fd)
{
	int connfd = *(int *)fd;
	uint8_t msg_type = 0, timeout = 0;
	int a = 5639, i = 0;
	SendMessage(connfd, &a, sizeof(a), PCKT_CONNECTION_RESPONSE);
	while(1) {
		//TODO update message has to be sent here, replace a with require function
		SendMessage(connfd, &a, sizeof(a), PCKT_UPDATE);
		RecieveMessage(connfd, &msg_type, &timeout);
		if ( timeout >= 5 )
		{
			NOTICE("<SERVER> Dropping player %d due to timeout\n", connfd);
			break;
		}
		if ( msg_type == PCKT_EVENT )
		{
			DEBUG("<SERVER> Received data from player %d\n", connfd);
			if ( getClientCount() == 5 ) { 
				decrClientCount(); 
				decrClientCount(); 
				decrClientCount(); 
				decrClientCount();
			}
		}
		//TODO update local structs with received data.
	}
    close(connfd);
}

