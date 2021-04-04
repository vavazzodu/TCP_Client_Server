/*
 * =====================================================================================
 *
 *       Filename:  mx_tcp_server.c
 *
 *    Description:  This file contains code of multiplex tcp server, which will be able to handle
 *                  more than one client. This server will handle it one by one.
 *
 *        Version:  1.0
 *        Created:  04/03/2021 05:12:28 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Karan Raval (), karanraval72@yahoo.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX 32
#define SERVER_PORT 2000
int monitor_fd_set[MAX];

void initialize_monitor_fd_set(void){
    int i=0;
    for(; i<MAX; i++)
        monitor_fd_set[i] = -1;
    return;
}
void add_to_monitor_fd_set(int sock_fd){
    int i=0;
    for(;i<MAX; i++) {
        if(monitor_fd_set[i] != -1)
            continue;
        monitor_fd_set[i] = sock_fd;    //empty location is found ; add sock_fd at that location and break the loop
        break;
    }
}
void re_init_fd_set(fd_set *fd_set_ptr){
    int i=0;
    FD_ZERO(fd_set_ptr); //clearing readfds
    for(;i<MAX;i++){
        if(monitor_fd_set[i] != -1)
            FD_SET(monitor_fd_set[i], fd_set_ptr);
    }
}
int get_max_fds(void){
    int i=0;
    int max=-1;
    for(;i<MAX;i++){
        if(monitor_fd_set[i] > max)
            max = monitor_fd_set[i];
    }
    return max;
}
void remove_fd_from_monitored_fd_set(int sock_fd){
    int i=0;
    for(; i<MAX; i++){
        if(monitor_fd_set[i] != sock_fd)
            continue;
        monitor_fd_set[i] = -1;
        break;
    }
}
void setup_mx_tcp_server(void)
{
    int main_fd = 0, 
        client_fd = 0,
        len = 0;
    struct sockaddr_in server_addr, 
                       client_addr;
    fd_set readfds;
     
    /* initialising fd set to -1 */
    initialize_monitor_fd_set();
    if((main_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Socket creating failed..\n");
        return;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = SERVER_PORT;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    len = sizeof(struct sockaddr);
    if((bind(main_fd, (struct sockaddr *)(&server_addr), len)) == -1){
        printf("Error in bind..\n");
        return;
    }
    if((listen(main_fd, 5)) == -1){
        printf("Error in listen...\n");
        return;
    }
    /*Adding master socket file desc in monitor FDs*/
    add_to_monitor_fd_set(main_fd);
    while(1){
        /* re-initialise FD_SET */
        re_init_fd_set(&readfds);
        printf("Blocked on select system call....\n");
        /*Will wait for clinet to send connection request */
        select(get_max_fds()+1, &readfds, NULL, NULL, NULL);
        /* Data has arrived on some FD present in monitor_fd_set, check on which FD data is arrived */
        if(FD_ISSET(main_fd, &readfds)){
            //Data is arrived on master socket file desc.It means new connection request is arrived, Accept it.
            printf("New connection received, 3-Way HS is completed..\n");
            client_fd = accept(main_fd, (struct sockaddr *)(&client_addr),&len);
            if(client_fd < 0){
                printf("Accept failed..\n");
                return;
            }
            //once we have communication FD we need to add it into monitored FD set.
            add_to_monitor_fd_set(client_fd);
            printf("Accepted the connection from clinet %s:%d...\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        }
        else{  /* Data arrived on some other clinet FD */
            int i = 0, comm_sock_fd = 1;
            int sent_recv_bytes = 0;
            int num = 0, result = 0;
            for(; i<MAX; i++){

                if(FD_ISSET(monitor_fd_set[i], &readfds)){/* Find clinet FD on which data has arrived*/
                    comm_sock_fd = monitor_fd_set[i];

                    sent_recv_bytes = recvfrom(comm_sock_fd, &num, sizeof(num),
                                         0, (struct sockaddr *)(&client_addr), &len);
                    printf("Server received %d bytes of data from client %s:%d..\n",sent_recv_bytes, inet_ntoa(client_addr.sin_addr),
                            ntohs(client_addr.sin_port));
                    if(sent_recv_bytes == 0){
                        close(comm_sock_fd);
                        remove_fd_from_monitored_fd_set(comm_sock_fd);
                        break; //Go to while loop and wait for data to arrive
                    }
                    if(num == 0){ //close the connection if data received is 0
                        printf("Data received is zero..\n");
                        close(comm_sock_fd);
                        remove_fd_from_monitored_fd_set(comm_sock_fd);
                        printf("Server closed connection with %s:%d as data recevied was 0..\n",
                                inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
                        break;
                    }
                    result = num + 10;
                    printf("Result = %d\n",result);
                    sent_recv_bytes = sendto(comm_sock_fd, (int *)&result, sizeof(result),
                                      0, (struct sockaddr *)(&client_addr),len);
                    printf("Server send reply of %d bytes..\n",sent_recv_bytes);
                }
            }
        }
    }
}
int main()
{    
    setup_mx_tcp_server();
    return 0;
}
