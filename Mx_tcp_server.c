/*
 * =====================================================================================
 *
 *       Filename:  TCP_server.c
 *
 *    Description:  This is multiplexing server file. please go through comments for more understanding.
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
#define MAX_FD 32

int monitor_fd[32];

static void
Init_monitor_fd(void){
    int i=0;
    for( ; i<MAX_FD ; i++){
        monitor_fd[i] = -1;
    }
}
static void
Add_to_monitor_fd(const int fd){
    int i = 0;
    for ( ; i<MAX_FD; i++){
        if(monitor_fd[i] != -1)
            continue;
        monitor_fd[i] = fd;
        break;
    }
}
static void
Copy_from_monitor_to_read(fd_set *fd){
    FD_ZERO(fd);
    int i = 0;
    for ( ; i < MAX_FD ; i++){
        if(monitor_fd[i] != -1)
            FD_SET(monitor_fd[i], fd);
    }
}
static void
Remove_fd_from_monitor_fd(const int fd){
    int i=0;
    for ( ; i < MAX_FD ; i++){
        if(monitor_fd[i] != fd)
            continue;
        monitor_fd[i] = -1;
        break;
    }
}
static int
get_max_fd(void){
    int max = -1;
    int i=0;
    for ( ; i<MAX_FD ; i++){
        if(monitor_fd[i] > max)
            max = monitor_fd[i];
    }
    return max;
}

void setup_tcp_server()
{
    //initialise variables.
    int master_sock_fd;
    int addr_len=0;
    int num, result;
    int send_recv_bytes=0;
    struct sockaddr_in server, //server data struct
                       client; //client data struct

    fd_set readfds;            //read file desc. where master socket fd and communication fds will stored
    int client_conn_fd=0;
    //initialise monitoring fd
    Init_monitor_fd();
    //Create master socket
    if((master_sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        printf("socket failed\n");
        exit(1);
    }
    //Fill server information
    server.sin_family = AF_INET;
    server.sin_port = SERVER_PORT;
    server.sin_addr.s_addr = INADDR_ANY;//Data reveiced on any interface of this server will be accepted.
    addr_len = sizeof(struct sockaddr);
    //Bind the server information with master socket
    if((bind(master_sock_fd, (struct sockaddr *)(&server), addr_len)) == -1){
        printf("Bind failed\n");
        return;
    }
    //tell OS to maintain a queue for incoming connection
    if((listen(master_sock_fd, 5)) == -1){
        printf("Listen failed\n");
        return;
    }
    //Add master fd to monitor fd
    Add_to_monitor_fd(master_sock_fd);
    while(1) { //server infinite loop for accepting new connection
        
        //copy fds from monitor fds to read fd
        Copy_from_monitor_to_read(&readfds);
        printf("Blocked at select system call...\n");
        //process will be blocked here untill new connection arrives
        select(get_max_fd()+1, &readfds, NULL, NULL, NULL);
        //process is unblocked, now we need to see on which file desc data has been arrived.
        if(FD_ISSET(master_sock_fd, &readfds)) {
            printf("New connection arrived and 3 way HS is done...\n");
            client_conn_fd = accept(master_sock_fd, (struct sockaddr *)(&client), &addr_len);
            if(client_conn_fd < 0){
                printf("accept failed..\n");
                return;
            }
            printf("Connection is accepted from client : %s:%d\n",
                   inet_ntoa(client.sin_addr),ntohs(client.sin_port));
            //Add new fd in monitor fd set
            Add_to_monitor_fd(client_conn_fd);
        }
        else {  //Data is received on one of the communication fd
            //loop through all fds in monitor fd and check on which data has been received
            int i;
            for(i=0;i<MAX_FD;i++) {
                if(FD_ISSET(monitor_fd[i], &readfds))
                    client_conn_fd = monitor_fd[i];
            //server is waiting for client to send data, hence the process will be blocked here.
            send_recv_bytes = recvfrom(client_conn_fd, &num, sizeof(num), 0,
                                      (struct sockaddr *)(&client), &addr_len);
            if(num == 0){
                printf("No data received..\n");
                //remove fd from monitoring fd set
                Remove_fd_from_monitor_fd(client_conn_fd);
                close(client_conn_fd);
                printf("Server has closed connection with client :%s:%d..\n",
                        inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                break;
            }
            printf("Server has received %d bytes data from client..\n",send_recv_bytes);
            printf("Data: %d\n",num);
            result = num + 10;
            send_recv_bytes = sendto(client_conn_fd, (char *)result, sizeof(result), 0,
                                   (struct sockaddr *)(&client), addr_len);
            if(send_recv_bytes)
                printf("server has send data %d \n", result);
            }
        }
    }
}

int main(int argc, char **argv)
{
    setup_tcp_server();
    return 0;
}
