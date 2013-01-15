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
#include <fcntl.h>

#include "logger.h"
#include "packets.h"
#include "socket_if.h"
#include "Graphics.h"

struct ConnectionResponse * fakeConnectionResponse()
{
	struct ConnectionResponse * response;
	response = (struct ConnectionResponse *) malloc(sizeof( struct ConnectionResponse) );
	response->id = 1;
	response->height = 24;
	response->width = 80;
	response->playerCount = 4;
	response->tailLength = 10;
	response->frameRate = 100;
	response->bulletSpeed = 5;
	response->bulletCooldown = 3;
	response->timeout = 5;
	return response;
}


static	World_t game;
int main(int argc, char *argv[])
{
	int counter=0;
	FILE *fd = fopen("./client.out", "w+");
	if ( fd != NULL )
		setOutputType(fd);
	else
		setOutputType(stderr);

	
	setLogLevel(LOG_LEVEL_DEBUG);

	uint8_t keep_alive = 1;
	uint8_t msg_type = 0, timeout = 0;
	int sockfd = 0, n = 0,c=0,xadd=0,yadd=1,BCD=0,running=1, startx=30,starty=30;
	char recvBuff[1024];
	uint8_t gameStarted = 0,fakeResponded=0;
	struct EventPlayer event;
	sleep_time.tv_sec  = 0;
    	sleep_time.tv_nsec = 450000000;

	memset(recvBuff, '0',sizeof(recvBuff));


#ifndef NO_SERVER
	if(argc != 3)
	{
		printf("\n Usage: %s <ip of server> <port of server>\n",argv[0]);
		return 1;
	}
#endif

	init_game();
#ifndef NO_SERVER
	keep_alive = ConnectToServer(argv[1], argv[2], &sockfd);
#else
	keep_alive = 1;
	
	conn_resp_t fakemsg;
    	fakemsg.id = 0;
	fakemsg.height = 24;  
	fakemsg.width = 80;
	fakemsg.playerCount = 5;
	fakemsg.tailLength = 10;
	fakemsg.frameRate = 60;
	fakemsg.bulletSpeed = 3;
	fakemsg.bulletCooldown = 3;
	fakemsg.timeout = 5;
	CreateClientWorld(&game, &fakemsg);
#endif

	while (keep_alive)
	{
#ifndef NO_SERVER
		void *buf = NULL;
		buf = RecieveMessage(sockfd, &msg_type, &timeout);
		
		if ( timeout == 5 ) {
			keep_alive = 0;
			ERROR("Connection to server timed out\n");
		} else {
			if(buf){
				switch(msg_type) {
					case 2:
						/* Player just connected to server;
							Message contains servers rules; */
						NOTICE("Connected to server, please stand by\n");
						CreateClientWorld(&game, buf);
						break;
					case 3:
						/* Update message received; calculate wait for button pressed */
						//DEBUG("Update information recieved\n" /*TODO add data here for debuging */);
						updateClientWorld(&game,buf);
						gameStarted = 1;
						break;
					default:
						keep_alive = 0;
						DEBUG("Trashed unhandeled message type:%d\n", msg_type);
						break;
				}
				free(buf);
			}

			if (gameStarted)
			{
#endif				
				init_game();
				c = wgetch(key_detecter);

				if ( c != ERR ) {
					ClientMove(c, &game);
				}		
				event.direction = (getSelf(&game))->direction;

				if ( c == ' ')
					event.shot = 1;
				else
					event.shot = 0;
#ifndef NO_SERVER
				SendMessage(sockfd, &event, sizeof(event), PCKT_EVENT);

			 	DEBUG("Sending update event { %d, %d } to server \n", event.direction, event.shot );
	    	}
#endif
			calculateField(&game);
			drawWorld(&game);
			//getchar();
			refresh();
			nanosleep(&sleep_time, NULL); 
			NOTICE("%c\n",c);
#ifndef NO_SERVER
		}
#endif
	
	}
	NOTICE("Disconnected from server\n");
	if (fd != NULL)
		fclose(fd);
	terminate_game();
	return 0;
}


