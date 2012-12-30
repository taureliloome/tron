#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * Info:
 * function connects to socket on address and port given and returns opened 
 * sockets file discriptor into sockfd
 * Input:
 * const char *ip - ipv4 address on which the server is located
 * int port       - port on the server
 * Output:
 * int *scokfd    - fd of opened socket will be stored here
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
        printf("\n Error : Could not create socket \n");
        return 0;
    }

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 0;
    }

    if( connect(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 0;
    }
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
        perror("unable to bind to listen socket: ");
	return 0;
    }
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
        return connfd;
    perror("unable to accept a connection");
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
    if ( sizeof(len) != write(sendfd, buf, len) )
    {
        perror("Unable to send msg");
		return 0;
    }
    return 1;
}

/**
 * Attempts to recieve a message from gived file discriptor;
 * Returns pointer to the recieved message payload, this has to be freed later.
 * Input:
 * int readfd 			- file discriptor from which to read data;
 * Output:
 * uint8_t *msg_type	- type of the recieved message;
 */
void *RecieveMessage(int readfd, uint8_t *msg_type)
{
    struct Header msg_hdr;
    size_t recieved = 0;
//TODO add time limit to atempts to read a message!
    while ( recieved != sizeof(msg_hdr) )
    {
        recieved += read(readfd, &msg_hdr + recieved, sizeof(msg_hdr) - recieved );
    }
	msg_hdr.length = ntohl(msg_hdr.length);
	*msg_type = msg_hdr.type;
	printf("Recieved message type: %d, len: %d\n", msg_hdr.type, msg_hdr.length );

    recieved = 0;
    void * buf = malloc(msg_hdr.length);
    while ( recieved != msg_hdr.length )
    {
        recieved += read(readfd, buf + recieved, sizeof(msg_hdr.length) - recieved );
		printf("%d/%d\n", recieved, msg_hdr.length);
    }
    return buf;    
}







