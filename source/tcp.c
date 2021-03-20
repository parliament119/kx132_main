/**
 * @file tcp.c
 * @author awa
 * @date 20-02-2021
 * 
 * @brief Contains functions for TCP-Communication with client.
 * 
 */

///cond
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
///endcond

#include <ringbuffer.h>
#include <macros_kx132.h>

#define PORT    60000
#define SA      struct sockaddr

static int sockfd, connfd;



bool tcp_server_init(void){
    uint32_t len;
    struct sockaddr_in servaddr, client;

    // Create socket for RaspberryPi
    // AF_INET : IPv4 address family, TCP-Protocol
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        printf("[tcp][error]Socket creation failed.\n");
        return false;
    }
    else{
        printf("[tcp] Socket successfully created.\n");
    }


    // Enable reuse of pending TCP-Socket/Port. Otherwise have to wait until timeout of socket.
    int sockReuse = 1;
    if ( (setsockopt(sockfd,SOL_SOCKET, SO_REUSEADDR, &sockReuse, sizeof(int))) < 0){
        printf("[tcp][error] SO_REUSEADDR failed.\n");
    }

    // bzero(&servaddr, sizeof(servaddr));
    memset(&servaddr, 0, sizeof(servaddr));


    // assing IPv4, Port
    servaddr.sin_family         = AF_INET;
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    servaddr.sin_port           = htons(PORT);

    // bind socket to IP and verification
    if((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0){
        printf("[tcp][error] Socket bind failed.\n");
        return false;
    }
    else{
        printf("[tcp] Socket successfully binded.\n");
    }

    // server ready to listen for client
    if((listen(sockfd, 5)) != 0){
        printf("[tcp][error] Listen failed.\n");
        return false;
    }
    else{
        printf("[tcp] TCP/IP Server listening.\n");
    }

    len = sizeof(client);

    // accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&client, &len);

    if(connfd < 0){
        printf("[tcp][error] TCP/IP Server accept failed.\n");
        return false;
    }
    else{
        printf("[tcp] TCP/IP Server accepted the client.\n");
    }

    return true;
}


void tcp_server_close(void){
    close(sockfd);
    printf("[tcp] TCP/IP Server closed.\n");
}


void tcp_send(int16_t* xyzFormatted){
    write(connfd, &xyzFormatted[X_INDEX], sizeof(int16_t));
    write(connfd, &xyzFormatted[Y_INDEX], sizeof(int16_t));
    write(connfd, &xyzFormatted[Z_INDEX], sizeof(int16_t));
}


void tcp_send_trig_buffer(int16_t **xyzFormatted, trigger_info_t *triggerInfo, int16_t *normalizedData){

    int16_t     tcpData         [NUMBER_OF_AXES];
    uint32_t    size            = triggerInfo->numberOfSamples;
    uint32_t    triggerIndex    = triggerInfo->numberOfSamples - triggerInfo->samplesAfterTrig;

    tcp_send(normalizedData);
    write(connfd, &size,         sizeof(uint32_t));
    write(connfd, &triggerIndex, sizeof(uint32_t));
    
    for(uint32_t i = 0; i < triggerInfo->numberOfSamples; i++){
        tcpData[X_AXIS] = xyzFormatted[X_INDEX][i];
        tcpData[Y_AXIS] = xyzFormatted[Y_INDEX][i];
        tcpData[Z_AXIS] = xyzFormatted[Z_INDEX][i];

        tcp_send(tcpData);
    }
}


void tcp_recv(char* data){
    read(connfd, data, 256);
}
