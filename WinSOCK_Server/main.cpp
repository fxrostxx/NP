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
#include <Messages.h>

using namespace std;

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "WinSOCK_FormatLastError.lib")

#define PORT "25575"
#define BUFFER_LENGTH 1500
#define MAX_CONNECTIONS 3

SOCKET sockets[MAX_CONNECTIONS] = {};
DWORD dwThreadIDs[MAX_CONNECTIONS] = {};
HANDLE hThreads[MAX_CONNECTIONS] = {};

INT gActiveClients = 0;

struct ClientParameters
{
	SOCKET clientSocket;
	addrinfo clientAddress;
};

VOID ClientHandle(SOCKET clientSocket);
//VOID Release(SOCKET clientSocket);
VOID ShowActiveClients();

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
	do
	{
		ShowActiveClients();
		struct sockaddr_in clientAddr;
		INT clientAddrlen = sizeof(clientAddr);
		clientAddr.sin_family = AF_INET;
		SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrlen);
		dwError = WSAGetLastError();
		if (clientSocket == INVALID_SOCKET) cout << "Accept failed: " << FormatLastError(dwError, szError) << endl;
		else
		{
			if (gActiveClients < MAX_CONNECTIONS)
			{
				sockets[gActiveClients] = clientSocket;
				hThreads[gActiveClients] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClientHandle, (LPSTR)sockets[gActiveClients], 0, &dwThreadIDs[gActiveClients]);
				++gActiveClients;
			}
			else
			{
				CHAR recvBuffer[BUFFER_LENGTH] = {};
				iResult = recv(clientSocket, recvBuffer, BUFFER_LENGTH, NULL);
				cout << recvBuffer << endl;
				iResult = send(clientSocket, DECLINE_MSG, strlen(DECLINE_MSG), NULL);
				shutdown(clientSocket, SD_BOTH);
				closesocket(clientSocket);
			}
		}
	} while (true);

	WaitForMultipleObjects(MAX_CONNECTIONS, hThreads, TRUE, INFINITE);

	closesocket(listenSocket);
	WSACleanup();

	return 0;
}

INT GetSlotIndex(DWORD dwID)
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (dwThreadIDs[i] == dwID) return i;
	}
}

VOID Shift(INT start)
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		sockets[i] = sockets[i + 1];
		dwThreadIDs[i] = dwThreadIDs[i + 1];
		hThreads[i] = hThreads[i + 1];
	}
	sockets[MAX_CONNECTIONS - 1] = NULL;
	dwThreadIDs[MAX_CONNECTIONS - 1] = NULL;
	hThreads[MAX_CONNECTIONS - 1] = NULL;
	--gActiveClients;
}

VOID ClientHandle(SOCKET clientSocket)
{
	sockaddr_in clientAddress;
	INT clientAddressLen = sizeof(clientAddress);
	getpeername(clientSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);

	CHAR szClientAddress[64] = {};
	CHAR* clientIP = inet_ntoa(clientAddress.sin_addr);
	INT clientPort = ntohs(clientAddress.sin_port);
	time_t now = time(nullptr);
	struct tm timeinfo;
	localtime_s(&timeinfo, &now);
	CHAR timeStr[64];
	sprintf(timeStr, "%i-%02i-%02i %02i:%02i:%02i", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	sprintf(szClientAddress, "%s %s:%d", timeStr, clientIP, clientPort);
	cout << szClientAddress << endl;

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
			cout << szClientAddress << " # " << recvBuffer << " (" << strlen(recvBuffer) << " bytes)" << endl;
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

	DWORD dwID = GetCurrentThreadId();
	Shift(GetSlotIndex(dwID));
	cout << szClientAddress << " left" << endl;

	iResult = shutdown(clientSocket, SD_BOTH);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR) cout << "Client shutdown failed. " << FormatLastError(dwError, szError) << endl;
	closesocket(clientSocket);
	ShowActiveClients();
	ExitThread(0);
	//Release(clientSocket);
}

//VOID Release(SOCKET clientSocket)
//{
//	for (int i = 0; i < MAX_CONNECTIONS; ++i)
//	{
//		if (clientSocket == sockets[i])
//		{
//			sockets[i] = NULL;
//			for (int j = i; sockets[j] || j < MAX_CONNECTIONS - 1; ++j)
//			{
//				sockets[j] = sockets[j + 1];
//				dwThreadIDs[j] = dwThreadIDs[j + 1];
//				hThreads[j] = hThreads[j + 1];
//			}
//		}
//	}
//	--g_ActiveClients;
//	ShowActiveClients();
//}

VOID ShowActiveClients()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hConsole, &info);
	COORD cursor = { 70, 1 };
	SetConsoleCursorPosition(hConsole, cursor);
	cout << "Number of connected clients: " << gActiveClients << endl;
	SetConsoleCursorPosition(hConsole, info.dwCursorPosition);
}