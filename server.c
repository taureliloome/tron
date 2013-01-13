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
#include "Graphics.h"

static void* clientHandler(void *fd);
static void waitForPlayers(int reqClientCount, int listenfd);
static int getPlayerIdFromConnFd(uint8_t connfd);

pthread_t accept_thread;
pthread_t derive_thread;

typedef enum {
	GS_STARTUP,
	GS_IN_PROGRESS,
	GS_ENDING,
	GS_MAX
} game_state_t;

static uint8_t game_state = GS_STARTUP;
uint8_t *playerTimeout = NULL;
uint32_t *playerFDs = NULL;
static World_t MyWorld;

//TODO: Send msg every 25 seconds. Error handling.
static void submitDirSrv(char *argv[]){
    // IP and PORT for Directory server
    const char* dirSrv[] = { "127.0.0.1", "1338" };
    int dir_alive = 1;  
    int dirsockfd = 0;
    char buf[255] = { '\0' };
    int len;
    strcpy(buf,"^A ");
    strcpy(buf+3,argv[1]);
    strcpy(buf+3+strlen(argv[1]),"$");
    len = strlen(buf);
    //printf("\n%s\n",buf); // debug of data buffer
    dir_alive = ConnectToServer(dirSrv[0], dirSrv[1], &dirsockfd);

    while(dir_alive){
        void *dirbuf = NULL;
        if ( len != write(dirsockfd, buf, len) )
        {
            perror(":(");
        }
    }
}

int main(int argc, char *argv[])
{
	FILE *fd = fopen("./server.out", "w+");
	if ( fd )
		setOutputType(fd);
	else
		setOutputType(stderr);

	setLogLevel(LOG_LEVEL_ALL);
    int listenfd = 0, playerCount = 5;
	playerTimeout = malloc(sizeof(uint8_t) * playerCount);
	playerFDs = malloc(sizeof(uint8_t) * playerCount);
	memset(playerTimeout, 0, sizeof(uint8_t) * playerCount);
	memset(playerFDs, 0, sizeof(uint8_t) * playerCount);

    //TODO: Put this function in right place
    submitDirSrv(argv);
	if ( argc != 2 )
	{
        printf("\n Usage: %s <port of server>\n",argv[0]);
		return 1;
	}
    CreateListenSocket(argv[1], &listenfd);
	
	waitForPlayers(playerCount, listenfd);

	close(listenfd);
	free(playerTimeout);
	free(playerFDs);
	return 0;
}

static void waitForPlayers(int reqClientCount, int listenfd)
{
	int connfd = 0;
    while(getClientCount() < reqClientCount)
    {
		connfd = ServerAcceptClient(&listenfd);
		playerFDs[getClientCount()-1] = connfd;

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
	uint32_t playerId = getPlayerIdFromConnFd(connfd);
	uint8_t msg_type = 0;
	int a = 5639, i = 0;
	void *upd_pkt;
	size_t upd_pkt_len = 0;
	SendMessage(connfd, &a, sizeof(a), PCKT_CONNECTION_RESPONSE);
	while(1) {
		RecieveMessage(connfd, &msg_type, &playerTimeout[playerId]);

		upd_pkt = getUpdateMessage(MyWorld, &upd_pkt_len);
		SendMessage(connfd, upd_pkt, upd_pkt_len, PCKT_UPDATE);
		if ( playerTimeout[playerId] >= 5 )
		{
			NOTICE("<SERVER> Dropping player %d due to timeout\n", connfd);
			break;
		}
		if ( msg_type == PCKT_EVENT )
		{
			DEBUG("<SERVER> Received data from player %d\n", connfd);
			if ( getClientCount() == 5 ) {
				break;
			}
		}
		//TODO update local structs with received data.
	}
    close(connfd);
}

static int getPlayerIdFromConnFd(uint8_t connfd)
{
	int id;
	for ( id = 0; id < getClientCount(); id++ )
	{
		if ( playerFDs[id] == connfd )
			return id;
	}
	return -1;
}
