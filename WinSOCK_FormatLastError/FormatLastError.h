#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <cstdio>

LPSTR FormatLastError(DWORD dwError, CHAR* szBuffer);