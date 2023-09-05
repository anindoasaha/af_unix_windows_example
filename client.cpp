#undef UNICODE
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <afunix.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_SOCKET "server.sock"

#define DEFAULT_BUFLEN 512

int __cdecl main(int argc, char **argv) 
{
  WSADATA wsaData;
  SOCKET ConnectSocket = INVALID_SOCKET;
  
  const char *sendbuf = "this is a test";
  int Result = 0;
  char recvbuf[DEFAULT_BUFLEN] = { 0 };
  int recvbuflen = DEFAULT_BUFLEN;
  SOCKADDR_UN RemoteSocket = { 0 };

  // Initialize Winsock
  Result = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (Result != 0) {
    printf("WSAStartup failed with error: %d\n", Result);
    return 1;
  }

 
  // Create a SOCKET for connecting to server
  ConnectSocket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (ConnectSocket == INVALID_SOCKET) {
    printf("socket failed with error: %ld\n", WSAGetLastError());
    WSACleanup();
    return 1;
  }

  // Connect to server.
  RemoteSocket.sun_family = AF_UNIX;
  strncpy_s(RemoteSocket.sun_path, sizeof RemoteSocket.sun_path, SERVER_SOCKET, (sizeof SERVER_SOCKET) - 1);
  
  Result = connect(ConnectSocket, (struct sockaddr *)&RemoteSocket, sizeof(RemoteSocket));
  if (Result == SOCKET_ERROR) {
    closesocket(ConnectSocket);
    ConnectSocket = INVALID_SOCKET;
    return 1;
  }

  if (ConnectSocket == INVALID_SOCKET) {
    printf("Unable to connect to server!\n");
    WSACleanup();
    return 1;
  }

  // Send an initial buffer
  Result = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
  if (Result == SOCKET_ERROR) {
    printf("send failed with error: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    return 1;
  }

  printf("Bytes Sent: %ld\n", Result);

  // shutdown the connection since no more data will be sent
  Result = shutdown(ConnectSocket, SD_SEND);
  if (Result == SOCKET_ERROR) {
    printf("shutdown failed with error: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    return 1;
  }

  // Receive until the peer closes the connection
  do {

    Result = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if ( Result > 0 )
      printf("Received %zu bytes: '%s'\n", strlen(recvbuf), recvbuf);
    else if ( Result == 0 )
      printf("Connection closed\n");
    else
      printf("recv failed with error: %d\n", WSAGetLastError());

  } while( Result > 0 );

  // cleanup
  closesocket(ConnectSocket);
  WSACleanup();

  return 0;
}