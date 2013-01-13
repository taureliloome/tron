#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "logger.h"

/**
  * Currently connected client count
  */
uint8_t clientCount = 0;

void setClientCount(uint8_t upd)
{
	clientCount = upd;
}

void incrClientCount()
{
	clientCount++;
}

void decrClientCount()
{
	clientCount--;
}

uint8_t getClientCount()
{
	return clientCount;
}

/**
 * Info:
 * function connects to socket on address and port given and returns opened 
 * sockets file discriptor into sockfd
 * Input:
 * const char *ip - ipv4 address on which the server is located
 * int port	   - port on the server
 * Output:
 * int *scokfd	- fd of opened socket will be stored here
 */
uint8_t ConnectToServer(const char *ip, const char *port, int *sockfd)
{
	int i_port = 0;
	struct sockaddr_in serv_addr;

	memset(&serv_addr, '0', sizeof(serv_addr));
	sscanf(port, "%d", &i_port);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(i_port);

	if((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		ERROR("Could not create socket: \n");
		return 0;
	}

	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
	{
		ERROR("inet_pton error occured\n");
		return 0;
	}

	if( connect(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		ERROR("Connect Failed \n");
		return 0;
	}
	NOTICE("Connected to server\n");
	return 1;
}

/**
 * Creates a socket and binds it to listen for any incoming messages from any ip
 * on speccified port
 *
 */
uint8_t CreateListenSocket(const char *port, int *listenfd)
{
	int i_port = 0;
	struct sockaddr_in serv_addr;

	memset(&serv_addr, '0', sizeof(serv_addr));
	sscanf(port, "%d", &i_port);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(i_port);

	*listenfd = socket(AF_INET, SOCK_STREAM, 0);	

	if ( bind(*listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) )
	{
		ERROR("Unable to bind to the listen socket: ");
		return 0;
	}
	NOTICE("Server started on with given port: %s\n", port);
	return 1;
}

/**
 * Wait 10 sec for a connection from client, if succesfull, then return fd of 
 * opened socket.
 */
int ServerAcceptClient(int * listenfd) 
{
	listen(*listenfd, 10);
	int connfd = accept(*listenfd, (struct sockaddr*)NULL, NULL);
	if ( connfd )
	{
		incrClientCount();
		DEBUG("Accepted client number: %d, connfd: %d \n", getClientCount(),  connfd);
		return connfd;
	}
	perror("Unable to accept a connection");
	return 0;
}

/**
 * Send message to given fd
 * Input:
 * int sendf			- file discriptor to which send the given data
 * void *buf			- pointer to the data buffer
 * size_t len 			- length of data that has to be sent
 * uint8_t type			- type of the message that will be sent
 */
uint8_t SendMessage(int sendfd, void * buf, size_t len, uint8_t msg_type)
{
	struct Header msg_hdr;
	msg_hdr.type = msg_type;
	msg_hdr.length = htonl(len);
	if ( sizeof(msg_hdr) != write(sendfd, &msg_hdr, sizeof(msg_hdr)) )
	{
		perror("Unable to send msg header");
		return 0;
	}
	if ( len != write(sendfd, buf, len) )
	{
		perror("Unable to send msg");
		return 0;
	}
	return 1;
}

/**
 * Attempts to recieve a message from gived file discriptor;
 * Returns pointer to the received message payload, this has to be freed later.
 * Input:
 * int readfd 			- file discriptor from which to read data;
 * Output:
 * uint8_t *msg_type	- type of the received message;
 */
void *RecieveMessage(int readfd, uint8_t *msg_type, uint8_t *timeout)
{
	struct Header msg_hdr;
	uint32_t received = 0;	
	struct timeval m_timeout;
	m_timeout.tv_sec = 0;
	m_timeout.tv_usec = 10000;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(readfd, &set);

	int res = select(readfd + 1, &set, NULL, NULL, &m_timeout );

	if ( res == 0 ){
		*timeout++;
		DEBUG2("Timed out, retry %d\n", *timeout);
		return NULL;
	}
	else if ( res == -1 ) {
		ERROR("Error in select:");
		return NULL;
	}
	*timeout = 0;
	while ( received != sizeof(msg_hdr) )
	{
		received += read(readfd, &msg_hdr + received, sizeof(msg_hdr) - received );
	}
	msg_hdr.length = ntohl(msg_hdr.length);
	*msg_type = msg_hdr.type;
	DEBUG("Received message type: %d, len: %d\n", msg_hdr.type, msg_hdr.length );

	received = 0;
	void * buf = malloc(msg_hdr.length);
	while ( received != msg_hdr.length )
	{
		received += read(readfd, buf + received, sizeof(msg_hdr.length) - received );
		DEBUG("%u/%u\n", received, msg_hdr.length);
	}
	return buf;	
}
