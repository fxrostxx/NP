#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <CommCtrl.h>
#include <iostream>
#include "resource.h"

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInsatnce, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	DialogBoxParam(hInsatnce, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)DlgProc, NULL);
	return 0;
}

LPSTR FormatCount(CHAR szBuffer[], DWORD dwCount);
LPSTR FormatAddress(CHAR szBuffer[], DWORD dwIPAddress);
VOID PrintInfo(HWND hwnd);

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hIPAddress = GetDlgItem(hwnd, IDC_IP_ADDRESS);
	HWND hIPMask = GetDlgItem(hwnd, IDC_IP_MASK);
	HWND hIPPrefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		SetFocus(GetDlgItem(hwnd, IDC_IP_ADDRESS));
		SendMessage(GetDlgItem(hwnd, IDC_SPIN_PREFIX), UDM_SETRANGE, 0, MAKEWORD(30, 0));
		break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_IP_ADDRESS:
		{
			DWORD dwIPAddress = 0;
			DWORD dwIPPrefix = 0;
			DWORD dwIPMask = 0;
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPAddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPAddress);
				if (FIRST_IPADDRESS(dwIPAddress) < 128)
				{
					dwIPMask = 0xFF000000U;
					dwIPPrefix = 8;
				}
				else if (FIRST_IPADDRESS(dwIPAddress) < 192)
				{
					dwIPMask = 0xFFFF0000U;
					dwIPPrefix = 16;
				}
				else if (FIRST_IPADDRESS(dwIPAddress) < 224)
				{
					dwIPMask = 0xFFFFFF00U;
					dwIPPrefix = 24;
				}
				SendMessage(hIPMask, IPM_SETADDRESS, 0, dwIPMask);
				CHAR szIPPrefix[3] = {};
				sprintf(szIPPrefix, "%i", dwIPPrefix);
				SendMessage(hIPPrefix, WM_SETTEXT, 0, (LPARAM)szIPPrefix);
				std::cout << dwIPMask << std::endl;
			}
			break;
		}
		case IDC_EDIT_PREFIX:
		{
			DWORD dwIPAddress = 0;
			DWORD dwIPPrefix = 0;
			DWORD dwIPMask = 0;
			if (HIWORD(wParam) == EN_CHANGE)
			{
				CHAR szIPPrefix[3] = {};
				SendMessage(hIPPrefix, WM_GETTEXT, 3, (LPARAM)szIPPrefix);
				DWORD dwIPPrefix = std::atoi(szIPPrefix);
				dwIPMask = UINT_MAX;
				dwIPMask = dwIPPrefix ? 0xFFFFFFFFU << (32 - dwIPPrefix) : 0;
				SendMessage(hIPMask, IPM_SETADDRESS, 0, dwIPMask);
			}
			break;
		}
		case IDC_BUTTON_RESET:
		{
			SendMessage(hIPAddress, IPM_CLEARADDRESS, 0, 0);
			SendMessage(hIPMask, IPM_CLEARADDRESS, 0, 0);
			SendMessage(hIPPrefix, WM_SETTEXT, 0, (LPARAM)"");
			break;
		}
		case IDOK: PrintInfo(hwnd); break;
		case IDCANCEL: EndDialog(hwnd, 0); break;
		}
		break;
	}
	case WM_NOTIFY:
	{
		NMHDR* pNMHDR = (NMHDR*)lParam;
		if (wParam == IDC_IP_MASK || wParam == IDC_IP_ADDRESS)
		{
			DWORD dwIPMask = 0;
			DWORD dwIPPrefix = 0;
			SendMessage(hIPMask, IPM_GETADDRESS, 0, (LPARAM)&dwIPMask);
			for (dwIPPrefix = 0; dwIPMask; dwIPPrefix++) dwIPMask <<= 1;
			CHAR szIPPrefix[3] = {};
			sprintf(szIPPrefix, "%i", dwIPPrefix);
			std::cout << szIPPrefix << std::endl;
			SendMessage(hIPPrefix, WM_SETTEXT, 0, (LPARAM)szIPPrefix);
		}
		break;
	}
	case WM_CLOSE: EndDialog(hwnd, 0); break;
	}
	return FALSE;
}

LPSTR FormatCount(CHAR szBuffer[], CONST CHAR szMessage[], DWORD dwCount)
{
	sprintf(szBuffer, "%s%i", szMessage, dwCount);
	return szBuffer;
}
LPSTR FormatAddress(CHAR szBuffer[], CONST CHAR szMessage[], DWORD dwIPAddress)
{
	sprintf
	(
		szBuffer,
		"%s%i.%i.%i.%i",
		szMessage,
		FIRST_IPADDRESS(dwIPAddress),
		SECOND_IPADDRESS(dwIPAddress),
		THIRD_IPADDRESS(dwIPAddress),
		FOURTH_IPADDRESS(dwIPAddress)
	);
	return szBuffer;
}
VOID PrintInfo(HWND hwnd)
{
	HWND hIPAddress = GetDlgItem(hwnd, IDC_IP_ADDRESS);
	HWND hIPMask = GetDlgItem(hwnd, IDC_IP_MASK);
	HWND HStaticInfo = GetDlgItem(hwnd, IDC_STATIC_INFO);
	DWORD dwIPAddress = 0;
	DWORD dwIPMask = 0;
	SendMessage(hIPAddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPAddress);
	SendMessage(hIPMask, IPM_GETADDRESS, 0, (LPARAM)&dwIPMask);
	DWORD dwNetworkAddress = dwIPAddress & dwIPMask;
	DWORD dwBroadcastAddress = dwIPAddress | ~dwIPMask;
	DWORD dwIPCount = dwBroadcastAddress - dwNetworkAddress + 1;
	DWORD dwHostCount = dwBroadcastAddress - dwNetworkAddress - 1;
	CHAR szNetworkAddress[1024]{};
	CHAR szBroadcastAddress[1024]{};
	CHAR szIPCount[1024]{};
	CHAR szHostCount[1024]{};
	CHAR szInfo[1024]{};
	sprintf
	(
		szInfo,
		"%s\n%s\n%s\n%s",
		FormatAddress(szNetworkAddress, "Network address: ", dwNetworkAddress),
		FormatAddress(szBroadcastAddress, "Braodcast address: ", dwBroadcastAddress),
		FormatCount(szIPCount, "Amount of IP addresses: ", dwIPCount),
		FormatCount(szHostCount, "Amount of Host addresses: ", dwHostCount)
	);
	SendMessage(HStaticInfo, WM_SETTEXT, 0, (LPARAM)szInfo);
}