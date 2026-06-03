#define _CRT_SECURE_NO_WARNINGS

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

int main()
{
	setlocale(LC_ALL, "");

	cout << "CLIENT" << endl;
	CHAR szError[256] = {};

	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
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

	iResult = getaddrinfo("127.0.0.1", PORT, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed: " << iResult << endl;
		WSACleanup();
		return 0;
	}

	SOCKET connectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (connectSocket == INVALID_SOCKET)
	{
		cout << "Socket creation error. " << FormatLastError(WSAGetLastError(), szError) << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	iResult = connect(connectSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Unable to connect to server. " << FormatLastError(WSAGetLastError(), szError) << endl;
		closesocket(connectSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	CHAR sendBuffer[BUFFER_LENGTH] = "Hello Server";
	CHAR recvBuffer[BUFFER_LENGTH] = {};
	do
	{
		iResult = send(connectSocket, sendBuffer, strlen(sendBuffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Send failed. " << FormatLastError(WSAGetLastError(), szError) << endl;
			closesocket(connectSocket);
			freeaddrinfo(result);
			WSACleanup();
			return 0;
		}
		cout << "Bytes sent: " << iResult << endl;

		iResult = recv(connectSocket, recvBuffer, BUFFER_LENGTH, 0);
		recvBuffer[iResult] = '\0';
		if (iResult > 0) cout << recvBuffer << " (" << iResult << " bytes)" << endl;
		else if (iResult == 0) cout << "Connection closed" << endl;
		else cout << "Receive failed. " << FormatLastError(WSAGetLastError(), szError) << endl;
		if (strcmp(recvBuffer, DECLINE_MSG) == 0)
		{
			system("pause");
			break;
		}
		SetConsoleCP(1251);
		cin.getline(sendBuffer, BUFFER_LENGTH);
		SetConsoleCP(866);
	} while (strcmp(sendBuffer, "exit") != 0);

	iResult = shutdown(connectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR) cout << "Shutdown failed. " << FormatLastError(WSAGetLastError(), szError) << endl;

	closesocket(connectSocket);
	freeaddrinfo(result);
	WSACleanup();

	return 0;
}