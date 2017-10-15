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

int iResult;
struct SocketParams
{
	int serverSocket;
	addrinfo *result;
};

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

addrinfo keep_the_result(addrinfo* result, int isItINPUTorOUTPUT)
{
	addrinfo theResult;

	if (isItINPUTorOUTPUT) // 1 = input, we want to put data
		theResult = *result;

	else //if (!isItINPUTorOUTPUT) // 0 == output we need the data
		;

	return theResult;
}

int my_create_socket()
{
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	addrinfo *result = NULL;
	iResult = getaddrinfo("192.168.1.104", DEFAULT_PORT, &hints, &result);
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

	///////////////////////////////////////		bind	///////////////////
	// Setup the TCP listening socket
	iResult = bind(serverSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	} ////////////////////////////////// 	end of	bind	///////////////
	
	freeaddrinfo(result);
	
	return serverSocket;
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

int my_connection(int clientSocket)
{
	///////////////////////////////////////		connection, snd, rsv ...	///////////////////
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			// iSendResult = send(clientSocket, recvbuf, iResult, 0);
			iSendResult = send(clientSocket, "Salam", 6, 0);
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

void my_cleanup(int clientSocket)
{
	closesocket(clientSocket);
	WSACleanup(); 
}

int main()
{
	iResult = my_init_socket();

	int serverSocket = my_create_socket(); // create the socket and " bind " 
	
	iResult = my_listen(serverSocket);

	iResult = my_accept(serverSocket);

	// No longer need server socket
	closesocket(serverSocket);

	SOCKET clientSocket = 111111111; // now try to	connect, snd, rsv ... (all is done in the my_connection func)
	
	int my_connection(clientSocket);

	// shutdown the connection since we're done
	iResult = my_shutdown(clientSocket);

	// cleanup
	my_cleanup(clientSocket);

	return 0;
}
