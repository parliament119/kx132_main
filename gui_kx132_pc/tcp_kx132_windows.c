/*
    Compile with:
        gcc -fPIC -shared -o tcp_kx132_windows.so tcp_kx132_windows.c -lws2_32

    @note any changes to this file need to be recompiled to take effect
*/

#include <winsock2.h>
#include <stdio.h>
#include <stdint.h>

SOCKET      clientSocket;
int16_t     tcpData [3];
uint32_t    tcp_uint32 [1];


int tcp_init(char* TCP_IP){
    WSADATA         wsaData;
    SOCKADDR_IN     ServerAddr;
    unsigned int    Port = 60000;                                                       // Port to connect to
    int             RetCode;

    WSAStartup(MAKEWORD(2,2), &wsaData);                                                // initialize Winsock
    printf("TCP: Winsock DLL status is %s.\n", wsaData.szSystemStatus);

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);                           // create socket for client connection
    if(clientSocket == INVALID_SOCKET)
    {
         printf("TCP: socket() failed! Error code: %ld\n", WSAGetLastError());
         WSACleanup();
         return -1;
    }
    else{
        printf("TCP: socket() is OK!\n"); 
    }


    ServerAddr.sin_family = AF_INET;                                                    // IPv4
    ServerAddr.sin_port = htons(Port);                                                  // Port #
    ServerAddr.sin_addr.s_addr = inet_addr(TCP_IP);                                     // IP-Address of server

    RetCode = connect(clientSocket, (SOCKADDR *) &ServerAddr, sizeof(ServerAddr));      // connect to server
    if(RetCode != 0)
    {
         printf("TCP: connect() failed! Error code: %ld\n", WSAGetLastError());
         closesocket(clientSocket);
         WSACleanup();
         return -1;
    }
    else
    {
         printf("TCP: connect() is OK, got connected...\n");
         printf("TCP: Ready for sending and/or receiving data...\n");
    }

    return 0;
}


void tcp_close(void){
    if(closesocket(clientSocket) != 0)
         printf("TCP: Cannot close socket. Error code: %ld\n", WSAGetLastError());
    else{
        printf("TCP: Closing socket...\n");
    }

    if(WSACleanup() != 0)
        printf("TCP: WSACleanup() failed!...\n\n");
    else{
        printf("TCP: WSACleanup() is OK...\n\n");
    }

    return;
}


void tcp_send(char *data){
    send(clientSocket, data, 256, 0);
    return;
}


int16_t* tcp_single_read(void){
    recv(clientSocket, (char*)&tcpData[0], sizeof(int16_t), 0);
    recv(clientSocket, (char*)&tcpData[1], sizeof(int16_t), 0);
    recv(clientSocket, (char*)&tcpData[2], sizeof(int16_t), 0);

    return tcpData;
}


uint32_t* tcp_read_uint32(void){
    recv(clientSocket, (char*)tcp_uint32, sizeof(uint32_t), 0);
    return tcp_uint32;
}

