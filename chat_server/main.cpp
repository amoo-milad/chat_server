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

#define DEFAULT_BUFLEN 256
#define DEFAULT_PORT "15000"

int my_init_socket();
int my_create_socket(const char* host, const char* port, SOCKET* serverSocket, addrinfo** myResult);
int my_bind(SOCKET* serverSocket, const struct sockaddr* ai_addrESS, int ai_addrlenTH, addrinfo** myResult);
int my_listen(SOCKET* serverSocket);
int my_accept(SOCKET serverSocket, SOCKET* clientSocket);
int my_recv(SOCKET clientSocket, char* recvbuf, int recvbuflen);
int my_send(SOCKET clientSocket, char* sendbuf, int iResult);
int my_shutdown(SOCKET clientSocket);
int my_cleanup(SOCKET clientSocket);

SOCKET theBindS(const char* host, const char* port); // Usage Func
int theSendResvRelation(SOCKET clientSocket); // Usage Func

int iResult = 0;

int my_init_socket()
{
	WSADATA wsaData;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	return iResult;
}

int my_create_socket(const char* host, const char* port, SOCKET* serverSocket, addrinfo** myResult)
{
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port

	iResult = getaddrinfo(host, port, &hints, myResult); //e.g 192.168.1.104
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	*serverSocket = socket((*myResult)->ai_family, (*myResult)->ai_socktype, (*myResult)->ai_protocol);
	if (*serverSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(*myResult);
		WSACleanup();
		return 1;
	}

	return 0;
}

// Setup the TCP listening socket
int my_bind(SOCKET* serverSocket, const struct sockaddr* ai_addrESS, int ai_addrlenTH, addrinfo** myResult)
{
	iResult = bind(*serverSocket, ai_addrESS, ai_addrlenTH);

	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(*myResult);
		closesocket(*serverSocket);
		WSACleanup();
		return 1;
	}
	
	return iResult;
}

int my_listen(SOCKET* serverSocket)
{
	iResult = listen(*serverSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(*serverSocket);
		WSACleanup();
		return 1;
	}

	return iResult;
}

int my_accept(SOCKET serverSocket, SOCKET* clientSocket)
{
	// Accept a client socket
	//SOCKET clientSocket = INVALID_SOCKET;
	*clientSocket = accept(serverSocket, NULL, NULL);
	if (*clientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	return iResult;
}

int my_recv(SOCKET clientSocket, char* recvbuf, int recvbuflen)
{
	iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
	
	return iResult;
}

int my_send(SOCKET clientSocket, char* sendbuf, int iResult)
{
	int iSendResult = send(clientSocket, sendbuf, iResult, 0);

	return iSendResult;
}

int my_shutdown(SOCKET clientSocket)
{
	// shutdown the connection since we're done
	iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	return iResult;
}

int my_cleanup(SOCKET clientSocket)
{
	closesocket(clientSocket);
	WSACleanup(); 

	return 0;
}

//////////////////////////////////////// My Usage Functions: /////////////////////////////
SOCKET theBindS(const char* host, const char* port) // Usage Func
{
	SOCKET serverSocket = INVALID_SOCKET;
	addrinfo* myResult = NULL;

	int initResult = my_init_socket();
	int createResult = my_create_socket(host, port, &serverSocket, &myResult);
	int bindResult = my_bind(&serverSocket, myResult->ai_addr, (int)myResult->ai_addrlen, &myResult);

	if (initResult == 0 && createResult == 0 && bindResult == 0)
		printf_s("theBindS compeleted!\n");
	else
	{
		printf_s("theBindS Failed!\n");
		printf_s("\t initResult   is: %d \n", initResult);
		printf_s("\t createResult is: %d \n",createResult);
		printf_s("\t bindResult   is: %d \n", bindResult);
	}

	freeaddrinfo(myResult);
	return serverSocket;
}

int theSendResvRelation(SOCKET clientSocket) // Usage Func
{
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	///int recvbuflen = DEFAULT_BUFLEN;
	char sendbuf[DEFAULT_BUFLEN] = "helloooooooo";

	// Receive until the peer shuts down the connection
	do {
		iResult = my_recv(clientSocket, recvbuf, (int)strlen(recvbuf));
		if (iResult > 0) {
			printf("\nBytes received: %d, which is %s\n", iResult, recvbuf);

			// Echo the buffer back to the sender
			///char* s1 = "Hellooooooooo ";
			///strcat(s1, recvbuf);
			iSendResult = my_send(clientSocket, sendbuf, (int)strlen(sendbuf));
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

	return iResult;
}

void check_result(int iResult, char* funcName)  // Usage Func
{
	if (iResult == 0)
		printf("check result: '%s'\t compeleted.\n", funcName);

	else if(iResult == 1)
		printf("check result: '%s'\t stoped!\n", funcName);

	else
		printf("check result: '%s'\t stoped by iResult: %d!\n", funcName, iResult);
}

//////////////////////////////////////// main: /////////////////////////////
int main()
{
	//char* funcName[20];

	const char* myHost = "0.0.0.0";
	const char* myPort = DEFAULT_PORT; // "15000"
	SOCKET serverSocket = theBindS(myHost, myPort); // my usage func

	iResult = my_listen(&serverSocket);
	check_result(iResult, "Listen");

	SOCKET clientSocket = INVALID_SOCKET;
	iResult = my_accept(serverSocket, &clientSocket);
	check_result(iResult, "Accept");

	// No longer need server socket
	closesocket(serverSocket);

	// now try to send and receive ...
	iResult = theSendResvRelation(clientSocket); // my usage func
	check_result(iResult, "SendResvRelation");

	// shutdown the connection since we're done
	iResult = my_shutdown(clientSocket);
	check_result(iResult, "Shutdown");

	// cleanup
	my_cleanup(clientSocket);

	return 0;
}
