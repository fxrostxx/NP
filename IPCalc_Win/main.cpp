#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
#include <iostream>

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInsatnce, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	DialogBoxParam(hInsatnce, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)DlgProc, NULL);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_IP_ADDRESS:
		{
			DWORD dwIPAddress = 0;
			DWORD dwIPMask = 0;
			HWND hIPAddress = GetDlgItem(hwnd, IDC_IP_ADDRESS);
			HWND hIPMask = GetDlgItem(hwnd, IDC_IP_MASK);
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPAddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPAddress);
				std::cout << FIRST_IPADDRESS(dwIPAddress) << std::endl;
				if (FIRST_IPADDRESS(dwIPAddress) < 128) dwIPMask = 0xFF000000U;
				else if (FIRST_IPADDRESS(dwIPAddress) < 192) dwIPMask = 0xFFFF0000U;
				else if (FIRST_IPADDRESS(dwIPAddress) < 224) dwIPMask = 0xFFFFFF00U;
				SendMessage(hIPMask, IPM_SETADDRESS, 0, dwIPMask);
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