// compile with: " gcc -fPIC -shared -o $NAME_OF_FILE$.so $NAME_OF_FILE$.c"
// Build with lib "-lws2_32"

// gcc -fPIC -shared -o tcp_multi_win.so tcp_multi_win.c -lws2_32

#include <winsock2.h>
#include <stdio.h>
#include <stdint.h>

SOCKET  clientSocket;
int16_t tcpData[3];
uint32_t tcp_uint32[1];


int tcp_init(int argc, char **argv)
{
    WSADATA              wsaData;
    // Server/receiver address
    SOCKADDR_IN          ServerAddr, ThisSenderInfo;
    // Server/receiver port to connect to
    unsigned int         Port = 60000;
    int  RetCode;
    // Be careful with the array bound, provide some checking mechanism...

    // Initialize Winsock version 2.2
    WSAStartup(MAKEWORD(2,2), &wsaData);
    printf("Client: Winsock DLL status is %s.\n", wsaData.szSystemStatus);

    // Create a new socket to make a client connection.
    // AF_INET = 2, The Internet Protocol version 4 (IPv4) address family, TCP protocol
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(clientSocket == INVALID_SOCKET)
    {
         printf("Client: socket() failed! Error code: %ld\n", WSAGetLastError());
         // Do the clean up
         WSACleanup();
         // Exit with error
         return -1;
    }
    else{
        printf("Client: socket() is OK!\n"); 
    }

    // Set up a SOCKADDR_IN structure that will be used to connect
    // to a listening server on port 8080.

    // IPv4
    ServerAddr.sin_family = AF_INET;
    // Port no.
    ServerAddr.sin_port = htons(Port);
    // The IP address
    ServerAddr.sin_addr.s_addr = inet_addr("100.200.150.42");

    // Make a connection to the server with socket clientSocket.
    RetCode = connect(clientSocket, (SOCKADDR *) &ServerAddr, sizeof(ServerAddr));
    if(RetCode != 0)
    {
         printf("Client: connect() failed! Error code: %ld\n", WSAGetLastError());
         // Close the socket
         closesocket(clientSocket);
         // Do the clean up
         WSACleanup();
         // Exit with error
         return -1;
    }
    else
    {
         printf("Client: connect() is OK, got connected...\n");
         printf("Client: Ready for sending and/or receiving data...\n");
    }

    // At this point you can start sending or receiving data on
    // the socket clientSocket.

    // Some info on the receiver side...
    getsockname(clientSocket, (SOCKADDR *)&ServerAddr, (int *)sizeof(ServerAddr));
    printf("Client: Receiver IP(s) used: %s\n", inet_ntoa(ServerAddr.sin_addr));
    printf("Client: Receiver port used: %d\n\n", htons(ServerAddr.sin_port));


    return 0;
}

void tcp_close(void){
    // When you are finished sending and receiving data on socket clientSocket,
    // you should close the socket using the closesocket API. We will
    // describe socket closure later in the chapter.
    if(closesocket(clientSocket) != 0)
         printf("Client: Cannot close \"clientSocket\" socket. Error code: %ld\n", WSAGetLastError());
    else{
        printf("Client: Closing \"clientSocket\" socket...\n");
    }

    // When your application is finished handling the connection, call WSACleanup.
    if(WSACleanup() != 0)
        printf("Client: WSACleanup() failed!...\n\n");
    else{
        printf("Client: WSACleanup() is OK...\n\n");
    }

    return;
}

void tcp_send(char *data){
    send(clientSocket, data, 256, 0);
    return;
}


int16_t* tcp_single_read(void){
    // recv( clientSocket , (char*)tcpData, 3*sizeof(int16_t), 0);
    
    recv( clientSocket , (char*)&tcpData[0], sizeof(int16_t), 0);
    recv( clientSocket , (char*)&tcpData[1], sizeof(int16_t), 0);
    recv( clientSocket , (char*)&tcpData[2], sizeof(int16_t), 0);

    return tcpData;
}

uint32_t* tcp_read_uint32(void){
    recv( clientSocket, (char*)tcp_uint32, sizeof(uint32_t), 0 );
    return tcp_uint32;
}

