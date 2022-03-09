#pragma once
#include <Windows.h>
#include <cstdlib>

PVOID GetData(HANDLE hProcess, DWORD dwSize);
BOOL FreeData(HANDLE hProcess, PVOID baseAddr, DWORD dwSize);