#define _CRT_SECURE_NO_WARNINGS

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

LPSTR FormatLastError(DWORD dwError, CHAR* szBuffer)
{
	LPSTR lpBuffer = NULL;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpBuffer,
		0,
		NULL
	);
	sprintf(szBuffer, "%i: %s", dwError, lpBuffer);
	LocalFree(lpBuffer);
	return szBuffer;
}

int main()
{
	setlocale(LC_ALL, "");

	cout << "CLIENT" << endl;

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
		cout << "Socket creation error: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	iResult = connect(connectSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		DWORD dwError = WSAGetLastError();
		CHAR szError[256] = {};
		cout << "Unable to connect to server. " << FormatLastError(dwError, szError) << endl;
		closesocket(connectSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	CHAR sendBuffer[BUFFER_LENGTH] = "Hello Server";
	CHAR recvBuffer[BUFFER_LENGTH] = {};
	iResult = send(connectSocket, sendBuffer, strlen(sendBuffer), 0);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Send failed:" << WSAGetLastError() << endl;
		closesocket(connectSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	cout << "Bytes sent: " << iResult << endl;

	do
	{
		iResult = recv(connectSocket, recvBuffer, BUFFER_LENGTH, 0);
		if (iResult > 0) cout << recvBuffer << " (" << iResult << " bytes)" << endl;
		else if (iResult == 0) cout << "Connection closed" << endl;
		else cout << "Receive failed: " << WSAGetLastError() << endl;
	} while (iResult > 0);

	iResult = shutdown(connectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Shutdown failed: " << WSAGetLastError() << endl;
		closesocket(connectSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	return 0;
}