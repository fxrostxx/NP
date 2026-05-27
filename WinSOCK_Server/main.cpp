#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

using namespace std;

#pragma comment(lib, "WS2_32.lib")

#define PORT "25575"
#define BUFFER_LENGTH 1500
#define MAX_CONNECTIONS 5

int main()
{
	setlocale(LC_ALL, "");

	cout << "SERVER" << endl;

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "WSAStartup failed: " << iResult << endl;
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
	if (iResult != 0)
	{
		cout << "getaddrinfo failed: " << iResult << endl;
		WSACleanup();
		return 0;
	}

	SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Socket creation error: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	iResult = bind(listenSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Bind failed: " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	freeaddrinfo(result);

	if (listen(listenSocket, MAX_CONNECTIONS) == SOCKET_ERROR)
	{
		cout << "Listen failed: " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	SOCKET clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) cout << "Accept failed: " << WSAGetLastError() << endl;

	CHAR recvBuffer[BUFFER_LENGTH] = {};
	CHAR sendBuffer[BUFFER_LENGTH] = {};
	INT iSendResult = 0;
	do
	{
		iResult = recv(clientSocket, recvBuffer, BUFFER_LENGTH, 0);
		if (iResult > 0)
		{
			cout << recvBuffer << " (" << strlen(recvBuffer) << " bytes)" << endl;
			iSendResult = send(clientSocket, recvBuffer, strlen(recvBuffer), 0);
			if (iSendResult == SOCKET_ERROR)
			{
				cout << "Send failed: " << WSAGetLastError() << endl;
				closesocket(clientSocket);
			}
			else cout << "Bytes sent: " << iSendResult << endl;
		}
		else if (iResult == 0) cout << "Connection closing" << endl;
		else
		{
			cout << "Receive failed: " << WSAGetLastError() << endl;
			closesocket(clientSocket);
		}
	} while (iResult > 0);

	iResult = shutdown(clientSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR) cout << "Client shutdown failed: " << WSAGetLastError() << endl;
	iResult = shutdown(listenSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR) cout << "Server shutdown failed: " << WSAGetLastError() << endl;

	closesocket(clientSocket);
	closesocket(listenSocket);
	WSACleanup();

	return 0;
}