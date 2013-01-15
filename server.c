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

#include "idgen.h"
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
static World_t ServerWorld;

void ReadConfig(FILE *fConfig)
{
	char *line = NULL ;
	size_t len = 0;
	getline(&line, &len, fConfig);
	ServerWorld.settings.height = getValue(line);
	getline(&line, &len, fConfig);
	ServerWorld.settings.width = getValue(line);
	getline(&line, &len, fConfig);
	ServerWorld.settings.playerCount = getValue(line);
	getline(&line, &len, fConfig);
	ServerWorld.settings.tailLength = getValue(line);
	getline(&line, &len, fConfig);
	ServerWorld.settings.frameRate = getValue(line);
	getline(&line, &len, fConfig);
	ServerWorld.settings.bulletSpeed = getValue(line);
	getline(&line, &len, fConfig);
	ServerWorld.settings.bulletCooldown = getValue(line);
	getline(&line, &len, fConfig);
	ServerWorld.settings.timeout = getValue(line); 
}

int getValue(char *line)
{
	int i=0, sum=0;
	while(line[i]!='\0'){
         if(line[i]< 48 || line[i] > 57){
             i++;
         }
         else{
             sum = sum*10 + (line[i] - 48);
             i++;
         }
    }
	return sum;
}

int main(int argc, char *argv[])
{
	FILE *fd = NULL; // fopen("./server.out", "w+");
	if ( fd )
		setOutputType(fd);
	else
		setOutputType(stderr);

	setLogLevel(LOG_LEVEL_DEBUG);
    int listenfd = 0, playerCount = 5;
	playerTimeout = malloc(sizeof(uint8_t) * playerCount);
	playerFDs = malloc(sizeof(uint8_t) * playerCount);
	memset(playerTimeout, 0, sizeof(uint8_t) * playerCount);
	memset(playerFDs, 0, sizeof(uint8_t) * playerCount);

	memset(&ServerWorld, 0, sizeof(ServerWorld));
	FILE *fConfig = fopen("./server.config", "r");
	if (fConfig )
		ReadConfig(fConfig);
	else {
        ERROR("\nConfiguration file \"server.config\" was not found.\n");
		return 1;
	}

	if ( argc != 2 )
	{
        ERROR("\n Usage: %s <port of server>\n",argv[0]);
		return 2;
	}

	init_world(&ServerWorld);
    //TODO: Enable when required sends messages to the list server
    //submitDirSrv(argv);

    CreateListenSocket(argv[1], &listenfd);
	
	waitForPlayers(ServerWorld.settings.playerCount, listenfd);

	close(listenfd);
	free(playerFDs);
	free(playerTimeout);
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

            /* PlayerID generation TODO: */
            int newId;
            newId = idGen(connfd, reqClientCount, idInit(reqClientCount));
            printf("newID is ---- %d\n", newId);
            /* --------- */

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
	uint8_t msg_type = 0;
	int a = 5639, i = 0;
	void *upd_pkt;
	size_t upd_pkt_len = 0;

	//Aizsutam saktonejo informaciju par servera iestatijumiem.
	conn_resp_t conn_resp = { 0 };
	memcpy(&conn_resp, &ServerWorld.settings, sizeof(conn_resp_t) );
	SendMessage(connfd, &conn_resp, sizeof(conn_resp), PCKT_CONNECTION_RESPONSE);
	conn_resp.id = 0; //getPlayerIdFromConnFd(connfd);

	while(1) {
		RecieveMessage(connfd, &msg_type, &playerTimeout[conn_resp.id]);
		upd_pkt = getUpdateMessage(&ServerWorld, &upd_pkt_len);
		SendMessage(connfd, upd_pkt, upd_pkt_len, PCKT_UPDATE);
		if ( playerTimeout[conn_resp.id] >= 5 )
		{
			NOTICE("<SERVER> Dropping player %d due to timeout\n", connfd);
			break;
		}
		if ( msg_type == PCKT_EVENT )
		{
			DEBUG("<SERVER> Received event from player %d\n", connfd);
			if ( getClientCount() == 5 ) {
				break;
			}
		}
		//TODO update local structs with received data.
	}
	DEBUG("<SERVER> Player %d session ended\n", conn_resp.id);
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
			ERROR("Unable to notify connection server");
        }
    }
}

