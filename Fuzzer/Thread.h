#pragma once
#include <Windows.h>
#include <Tlhelp32.h>

#pragma pack(1)
typedef struct _CONTEXTLIST
{
	struct _CONTEXTLIST *next;
	DWORD dwThreadId;
	CONTEXT ctx;
} CONTEXTLIST, *PCONTEXTLIST;
#pragma pack()

BOOL SaveThreadContext(DWORD dwPid);		// 将需要的线程CONTEXT保存起来
BOOL RestoreThreadContext();				// 恢复被保存的CONTEXT