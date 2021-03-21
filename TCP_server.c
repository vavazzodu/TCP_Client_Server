/*
 * =====================================================================================
 *
 *       Filename:  TCP_server.c
 *
 *    Description:  This is server file. please go through comments for more understanding.
 *
 *        Version:  1.0
 *        Created:  06/20/2020 12:50:49 AM
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
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <memory.h>
#include <netdb.h>

#define SERVER_PORT 2000
void setup_tcp_server(void);

void setup_tcp_server()
{
    //step-1, initialise variables.
    int master_sock_fd;
    int addr_len=0;
    int num,results;
    int send_recv_bytes=0;
    struct sockaddr_in server, client; //server and client data struct
    fd_set readfds;            //read file desc. where master socket fd and communication fds will stored
    int client_conn_fd=0;
    //step 2 create master socket
    if((master_sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        printf("socket failed\n");
        exit(1);
    }
    //step 3 fill server information
    server.sin_family = AF_INET;
    server.sin_port = SERVER_PORT;
    server.sin_addr.s_addr = INADDR_ANY;//Data reveiced on any interface of this server will be accepted.
    addr_len = sizeof(struct sockaddr);
    //step4 Bind the server information with master socket
    if((bind(master_sock_fd, (struct sockaddr *)(&server), addr_len)) == -1){
        printf("Bind failed\n");
        return;
    }
    //step 5 tell OS to maintain a queue for incoming connection
    if((listen(master_sock_fd, 5)) == -1){
        printf("Listen failed\n");
        return;
    }
    while(1) { //server infinite loop for accepting new connection
        //step 6 clearing fd set and setting 
        FD_ZERO(&readfds);
        FD_SET(master_sock_fd, &readfds);
        printf("Blocked at select system call...\n");
        //process will be blocked here untill new connection arrives
        select(master_sock_fd+1, &readfds, NULL, NULL, NULL);
        perror("select");
        //process is unblocked, now we need to see on which file desc data has been arrived.
        if(FD_ISSET(master_sock_fd, &readfds)){ //If data is arrived on master sock fd then it will be for new connection
            //data was for new connection. accept it
            printf("New connection arrived and 3 way HS is done...\n");
            client_conn_fd = accept(master_sock_fd, (struct sockaddr *)(&client), &addr_len);
            if(client_conn_fd < 0){
                printf("accept failed..\n");
                return;
            }
            printf("Connection is accepted from client : %s:%d\n",
                   inet_ntoa(client.sin_addr),ntohs(client.sin_port));
            while(1){
                printf("Waiting for data//\n");
                //server is waiting for client to send data, hence the process will be blocked here.
                memset((int *)&num,0,sizeof(num));
                send_recv_bytes = recvfrom(client_conn_fd, &num, sizeof(num), 0,
                                           (struct sockaddr *)(&client), &addr_len);
                if(num == 0){
                    printf("Server has received 0, hence closing the connection..\n");
                    close(client_conn_fd);
                    printf("Server has closed connection with client :%s:%d..\n",
                             inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                    break;
                }
                printf("Server has received %d data from client..\n",send_recv_bytes);
                results = num +10;
                send_recv_bytes = sendto(client_conn_fd, &results, sizeof(results), 0,
                                         (struct sockaddr *)(&client), addr_len);
               /* if(send_recv_bytes)
                   printf("server has send data %d bytes\n",send_recv_bytes);
                close(client_conn_fd);
                break;*/
            }
        }
    }
}
int main(int argc, char **argv)
{
    setup_tcp_server();
    return 0;
}
