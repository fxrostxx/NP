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

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD dwIPAddress = 0;
	DWORD dwIPMask = 0;
	DWORD dwIPPrefix = 0;
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
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPAddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPAddress);
				if (FIRST_IPADDRESS(dwIPAddress) < 128)
				{
					dwIPMask = 0xFF000000U;
					//dwIPPrefix = 8;
				}
				else if (FIRST_IPADDRESS(dwIPAddress) < 192)
				{
					dwIPMask = 0xFFFF0000U;
					//dwIPPrefix = 16;
				}
				else if (FIRST_IPADDRESS(dwIPAddress) < 224)
				{
					dwIPMask = 0xFFFFFF00U;
					//dwIPPrefix = 24;
				}
				SendMessage(hIPMask, IPM_SETADDRESS, 0, dwIPMask);
				CHAR szIPPrefix[3] = {};
				sprintf(szIPPrefix, "%i", dwIPPrefix);
				SendMessage(hIPPrefix, WM_SETTEXT, 0, (LPARAM)szIPPrefix);
				//std::cout << dwIPMask << std::endl;
			}
			break;
		}
		case IDC_IP_MASK:
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPMask, IPM_GETADDRESS, 0, (LPARAM)&dwIPMask);
				dwIPMask &= 0xFFFFFFFC;
				if (HIWORD(wParam) == EN_CHANGE) dwIPMask = dwIPPrefix ? 0xFFFFFFFFU << (32 - dwIPPrefix) : 0;
				CHAR szIPPrefix[3] = {};
				sprintf(szIPPrefix, "%i", dwIPPrefix);
				std::cout << szIPPrefix << std::endl;
				SendMessage(hIPPrefix, WM_SETTEXT, 0, (LPARAM)szIPPrefix);
				//if(HIWORD(wParam) == EN_KILLFOCUS)SendMessage(hIPMask, IPM_SETADDRESS, 0, dwIPAddress);
			}
			break;
		}
		case IDOK: break;
		case IDCANCEL: EndDialog(hwnd, 0); break;
		}
		break;
	}
	case WM_CLOSE: EndDialog(hwnd, 0); break;
	}
	return FALSE;
}