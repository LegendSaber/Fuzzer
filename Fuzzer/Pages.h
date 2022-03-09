#pragma once
#include <Windows.h>
#include "auxiliary.h"

#pragma pack(1)
typedef struct _MEMORYBLOCKSLIST
{
	struct _MEMORYBLOCKSLIST *next;
	PVOID read_buf;
	MEMORY_BASIC_INFORMATION mbi;
}MEMORYBLOCKSLIST, *PMEMORYBLOCKSLIST;
#pragma pack()

BOOL SavePages(HANDLE hProcess);
BOOL RestorePages(HANDLE hProcess);