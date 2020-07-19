/*
 * =====================================================================================
 *
 *       Filename:  TCP_client.c
 *
 *    Description:  This is client file. please read comments for more understanding.
 *
 *        Version:  1.0
 *        Created:  06/20/2020 06:53:36 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Karan Raval (), karanraval72@yahoo.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>

#define SERVER_PORT 2000
#define SERVER_IP "127.0.0.1"

//char data_buffer[1024];

void setup_client()
{
    int sockfd, num, num_1;
    int addr_len= sizeof(struct sockaddr);
    int send_recv_bytes = 0;
    struct sockaddr_in server;
    
    server.sin_port = SERVER_PORT;
    struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP);
    server.sin_addr = *((struct in_addr *)host->h_addr);
    server.sin_family = AF_INET;
    
    //step 1 create socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("Socket creation done..\n");
    //step 2 connect to the server specified by port no and ip address
    connect(sockfd, (struct sockaddr *)(&server), addr_len);
    printf("Successfully connected to server..\n");
    while(1){
        printf("Enter the data\n");
        scanf("%d",&num);
        send_recv_bytes = sendto(sockfd, &num, sizeof(int),
                           0, (struct sockaddr *)(&server), addr_len);
        printf("Client has send %d bytes\n",send_recv_bytes);
        send_recv_bytes = recvfrom(sockfd, &num_1, sizeof(int),
                           0, (struct sockaddr *)(&server), &addr_len);
        printf("%d data recevied from server\n",send_recv_bytes);
        printf("Data: %d is received from server\n", num_1);
    }
          
}
int main(int argc, char **argv)
{
    printf("setting up client..\n");
    setup_client();
    printf("Process over..\n");
    return 0;
}
