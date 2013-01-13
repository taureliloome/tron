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
#include <libconfig.h>

#include "logger.h"
#include "packets.h"
#include "socket_if.h"
#include "Graphics.h"

static	World_t game;
int main(int argc, char *argv[])
{
	FILE *fd = fopen("./client.out", "w+");
	if ( fd != NULL )
		setOutputType(fd);
	else
		setOutputType(stderr);

	setLogLevel(LOG_LEVEL_ALL);

	uint8_t keep_alive = 1;
	uint8_t msg_type = 0, timeout = 0;
	int sockfd = 0, n = 0,c=0,xadd=0,yadd=1,BCD=0,running=1, startx=30,starty=30;
	char recvBuff[1024];
	uint8_t gameStarted = 0;
	struct EventPlayer event;
	sleep_time.tv_sec  = 0;
    	sleep_time.tv_nsec = 450000000;

	memset(recvBuff, '0',sizeof(recvBuff));


	if(argc != 3)
	{
		printf("\n Usage: %s <ip of server> <port of server>\n",argv[0]);
		return 1;
	}

	init_game();
#ifdef SERVER_ACTIVE
	keep_alive = ConnectToServer(argv[1], argv[2], &sockfd);
#endif
	while (keep_alive)
	{
		
		void *buf = NULL;
#ifdef SERVER_ACTIVE
		buf = RecieveMessage(sockfd, &msg_type, &timeout);
#endif
		if ( timeout == 5 ) {
			keep_alive = 0;
			ERROR("Connection to server timed out\n");
		} else {
			if(buf){
				switch(msg_type) {
					case 2:
						/* Player just connected to server;
							Message contains servers rules;
						*/
						NOTICE("Connected to server, please stand by\n");
//						CreateClientWorld(&game, buf);
						break;
					case 3:
						/* Update message received; calculate wait for button pressed */
						DEBUG("Update information recieved\n" /*TODO add data here for debuging */);
//						UpdateWorld(buf);
						gameStarted = 1;
						break;
					default:
						keep_alive = 0;
						DEBUG("Trashed unhandeled message type:%d\n", msg_type);
						break;
				}
				free(buf);
			}
#ifndef SERVER_ACTIVE
			gameStarted = 1;
#endif
			if (gameStarted)
			{
				init_game();
				c = wgetch(key_detecter); 
				//if ( c != ERR ) {
					//TODO pass c to world
					//TODO send
#ifdef SERVER_ACTIVE
					event.direction = DIR_UP;// (getSelf(&game))->direction;
					if ( c == ' ')
						event.shot = 1;
					else
						event.shot = 0;
					SendMessage(sockfd, &event, sizeof(event), PCKT_EVENT);
					DEBUG("Sending update event { %d, %d } to server \n", event.direction, event.shot );
#endif
				//}
		    }

			drawWorld(&game);			
			refresh();
		}
	}
	NOTICE("Disconnected from server\n");
	if (fd != NULL)
		fclose(fd);
	terminate_game();
	return 0;
}
