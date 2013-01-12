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
#include "World.h"
#include "Moving.h"

int main(int argc, char *argv[])
{
	World game;
	uint8_t keep_alive = 1;
	uint8_t msg_type = 0, timeout = 0;
	int sockfd = 0, n = 0,c=0,xadd=0,yadd=1,BCD=0,running=1;
	char recvBuff[1024];
	bool DataReceived;
	sleep_time.tv_sec  = 0;
    sleep_time.tv_nsec = 450000000;

	memset(recvBuff, '0',sizeof(recvBuff));

	if(argc != 3)
	{
		printf("\n Usage: %s <ip of server> <port of server>\n",argv[0]);
		return 1;
	}
	init_game();
	//ConnectToServer(argv[1], argv[2], &sockfd);

	while (keep_alive)
	{
		
		//void *buf = RecieveMessage(sockfd, &msg_type, &timeout);
		void *buf;
		if ( timeout == 5 ) {
			keep_alive = 0;
			printf("Connection to server timed out\n");
		} else {
			DataReceived=false;
			//if ( buf ) 
			if(false){
				switch(msg_type) {
					case 2:
						printf("connection accepted - execute connection sequences!!!!!!\n");
						CreateClientWorld(&game,buf);
						break;
					case 3:
						printf("new intelengence has been recieved, update our databases\n");
						UpdateWorld(buf);
						DataReceived = true;
						break;
					default:
						keep_alive = 0;
						printf("we are burried in trash T_T\n");
						break;
				}
				free(buf);
			}
			DataReceived=true;//tests of controls only
			if (DataReceived)
			{
				init_game();
				c = wgetch(key_detecter);
				mvaddch(10, 10, c);
				nanosleep(&sleep_time, NULL); 
				switch (c) {
				    case 'w':
				        yadd = -1;
						xadd = 0;
				        break;
				    case 's':
				        yadd = 1;
						xadd = 0;
				        break;
				    case 'a':
				        xadd = -1;
						yadd = 0;
				        break;
				    case 'd':
				        xadd = 1;
						yadd = 0;
				        break;
					case ' ':
						running = 2;
						if (BCD == 0)
							BCD = game.bulletCoolDown;
						break;
				    case 'q':
				        running = 0;
				        break;
				}
		    }
			
			refresh();
			//TODO: replace int a with proper EVENT structure!!!
			int a = 5432;
			//SendMessage(sockfd, &a, sizeof(a), PCKT_EVENT);
		}
	}
	terminate_game();
	return 0;
}
