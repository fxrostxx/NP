#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <FormatLastError.h>

using namespace std;

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "WinSOCK_FormatLastError.lib")

#define PORT "25575"
#define BUFFER_LENGTH 1500
#define MAX_CONNECTIONS 5

SOCKET sockets[MAX_CONNECTIONS] = {};
DWORD dwThreadIDs[MAX_CONNECTIONS] = {};
HANDLE hThreads[MAX_CONNECTIONS] = {};

struct ClientParameters
{
	SOCKET clientSocket;
	addrinfo clientAddress;
};

VOID ClientHandle(SOCKET clientSocket);

INT main()
{
	setlocale(LC_ALL, "");

	cout << "SERVER" << endl;
	DWORD dwError = 0;
	CHAR szError[256] = {};

	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	dwError = WSAGetLastError();
	if (iResult != 0)
	{
		cout << "WSAStartup failed. " << FormatLastError(dwError, szError) << iResult << endl;
		return 0;
	}

	struct addrinfo hints;
	struct addrinfo* result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	dwError = WSAGetLastError();
	if (iResult != 0)
	{
		cout << "getaddrinfo failed. " << FormatLastError(dwError, szError) << endl;
		WSACleanup();
		return 0;
	}

	SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	dwError = WSAGetLastError();
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Socket creation error. " << FormatLastError(dwError, szError) << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	iResult = bind(listenSocket, result->ai_addr, result->ai_addrlen);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR)
	{
		cout << "Bind failed: " << FormatLastError(dwError, szError) << endl;
		closesocket(listenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	freeaddrinfo(result);

	if (listen(listenSocket, MAX_CONNECTIONS) == SOCKET_ERROR)
	{
		dwError = WSAGetLastError();
		cout << "Listen failed: " << FormatLastError(dwError, szError) << endl;
		closesocket(listenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	INT i = 0;
	do
	{
		struct sockaddr_in clientAddr;
		INT clientAddrlen = sizeof(clientAddr);
		clientAddr.sin_family = AF_INET;
		SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrlen);
		dwError = WSAGetLastError();
		if (clientSocket == INVALID_SOCKET) cout << "Accept failed: " << FormatLastError(dwError, szError) << endl;
		else
		{
			//ClientHandle(clientSocket);
			if (i < MAX_CONNECTIONS)
			{
				sockets[i] = clientSocket;
				hThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClientHandle, (LPSTR)sockets[i], 0, &dwThreadIDs[i]);
				++i;
			}
			CHAR* clientIP = inet_ntoa(clientAddr.sin_addr);
			INT clientPort = ntohs(clientAddr.sin_port);
			time_t now = time(nullptr);
			struct tm timeinfo;
			localtime_s(&timeinfo, &now);
			CHAR timeStr[64];
			sprintf(timeStr, "%i-%02i-%02i %02i:%02i:%02i", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
			cout << timeStr << " Accepted client from " << clientIP << ":" << clientPort << endl;
		}
	} while (true);

	//iResult = shutdown(listenSocket, SD_RECEIVE);
	//dwError = WSAGetLastError();
	//if (iResult == SOCKET_ERROR) cout << "Server shutdown failed: " << FormatLastError(dwError, szError) << endl;

	closesocket(listenSocket);
	WSACleanup();

	return 0;
}

VOID ClientHandle(SOCKET clientSocket)
{
	sockaddr_in clientAddress;
	INT clientAddressLen = sizeof(clientAddress);
	getpeername(clientSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);

	CHAR* clientIP = inet_ntoa(clientAddress.sin_addr);
	INT clientPort = ntohs(clientAddress.sin_port);
	time_t now = time(nullptr);
	struct tm timeinfo;
	localtime_s(&timeinfo, &now);
	CHAR timeStr[64];
	sprintf(timeStr, "%i-%02i-%02i %02i:%02i:%02i", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	cout << timeStr << " Accepted client from " << clientIP << ":" << clientPort << endl;

	INT iResult = 0;
	DWORD dwError = 0;
	CHAR szError[256] = {};

	CHAR recvBuffer[BUFFER_LENGTH] = {};
	CHAR sendBuffer[BUFFER_LENGTH] = {};
	INT iSendResult = 0;
	do
	{
		iResult = recv(clientSocket, recvBuffer, BUFFER_LENGTH, 0);
		recvBuffer[iResult] = '\0';
		dwError = WSAGetLastError();
		if (iResult > 0)
		{
			cout << recvBuffer << " (" << strlen(recvBuffer) << " bytes)" << endl;
			iSendResult = send(clientSocket, recvBuffer, strlen(recvBuffer), 0);
			dwError = WSAGetLastError();
			if (iSendResult == SOCKET_ERROR)
			{
				cout << "Send failed: " << FormatLastError(dwError, szError) << endl;
				closesocket(clientSocket);
			}
			else cout << "Bytes sent: " << iSendResult << endl;
		}
		else if (iResult == 0) cout << "Connection closing" << endl;
		else
		{
			cout << "Receive failed: " << FormatLastError(dwError, szError) << endl;
			closesocket(clientSocket);
		}
	} while (iResult > 0);

	iResult = shutdown(clientSocket, SD_BOTH);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR) cout << "Client shutdown failed. " << FormatLastError(dwError, szError) << endl;
	closesocket(clientSocket);
}