#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * Input:
 * const char *ip - ipv4 address on which the server is located
 * int *scokfd    - fd of opened socket will be stored here
 */
uint8_t ConnectToServer(const char *ip, int *sockfd) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));

    if((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 0;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 0;
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 0;
    }
    return 1;
}


