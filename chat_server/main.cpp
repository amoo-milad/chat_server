// chat_server.cpp // from (c) https://msdn.microsoft.com/en-us/library/windows/desktop/ms737593(v=vs.85).aspx
#include "stdafx.h"

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "15000"

int my_init_socket();
int my_create_socket(const char* host, const char* port);
int my_bind(int serverSocket, const struct sockaddr* ai_addrESS, int ai_addrlenTH);
int my_listen(int serverSocket);
int my_accept(int serverSocket);
int my_recv(int clientSocket, char* recvbuf, int recvbuflen, int zero);
int my_send(int clientSocket, char* recvbuf, int iResult, int zero);
int my_shutdown(int clientSocket);
int my_cleanup(int clientSocket);

int theBindS(const char* host, const char* port); // Usage Func
int theSendResvRelation(int clientSocket); // Usage Func

int iResult = 0;
addrinfo* myResult = NULL;	// global

int my_init_socket()
{
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	return iResult;
}

int my_create_socket(const char* host, const char* port)
{
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	addrinfo *result = NULL;
	iResult = getaddrinfo(host, port, &hints, &result); //e.g 192.168.1.104
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET serverSocket = INVALID_SOCKET;
	// Create a SOCKET for connecting to server
	serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (serverSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	myResult = result; // made it global for other functions like 'bind' and 'freeaddrinfo(myResult)'
	
	return serverSocket;
}

// Setup the TCP listening socket
int my_bind(int serverSocket, const struct sockaddr* ai_addrESS, int ai_addrlenTH)
{
	iResult = bind(serverSocket, ai_addrESS, ai_addrlenTH);

	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(myResult);
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
}

int my_listen(int serverSocket)
{
	iResult = listen(serverSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
}

int my_accept(int serverSocket)
{
	// Accept a client socket
	SOCKET clientSocket = INVALID_SOCKET;
	clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
}

int my_recv(int clientSocket, char* recvbuf, int recvbuflen, int zero)
{
	iResult = recv(clientSocket, recvbuf, recvbuflen, zero);
	
	return iResult;
}

int my_send(int clientSocket, char* recvbuf, int iResult, int zero)
{
	int iSendResult = send(clientSocket, recvbuf, iResult, zero);

	return iSendResult;
}

int my_shutdown(int clientSocket)
{
	// shutdown the connection since we're done
	iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
}

int my_cleanup(int clientSocket)
{
	closesocket(clientSocket);
	WSACleanup(); 

	return 0;
}

//////////////////////////////////////// My Usage Functions: /////////////////////////////
int theBindS(const char* host, const char* port) // Usage Func
{
	// i've dropped the first parameter(int serverSocket) because it was unnecessary.
	int initResult = my_init_socket();
	int serverSocket = my_create_socket(host, port);
	int bindResult = my_bind(serverSocket, myResult->ai_addr, (int)myResult->ai_addrlen); //or host and port?

	printf("initResult is: %d and bindResult is: %d\n", initResult, bindResult);
	return serverSocket;
}

int theSendResvRelation(int clientSocket) // Usage Func
{
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Receive until the peer shuts down the connection
	do {
		iResult = my_recv(clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = my_send(clientSocket, "Salam", iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);
}

//////////////////////////////////////// main: /////////////////////////////
int main()
{
	const char* myHost = "0.0.0.0";
	const char* myPort = DEFAULT_PORT; // "15000"
	int serverSocket = theBindS(myHost, myPort); // my usage func

//	freeaddrinfo(myResult);  // why does it error when i uncomment it???

	iResult = my_listen(serverSocket);

	iResult = my_accept(serverSocket);

	// No longer need server socket
	closesocket(serverSocket);

	SOCKET clientSocket = 666; 
	
	// now try to send and receive ...
	iResult = theSendResvRelation(clientSocket); // my usage func

	// shutdown the connection since we're done
	iResult = my_shutdown(clientSocket);

	// cleanup
	my_cleanup(clientSocket);

	return 0;
}
