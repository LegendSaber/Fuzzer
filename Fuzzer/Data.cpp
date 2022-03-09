#include "Data.h"
#include "auxiliary.h"

// 本进程中申请内存空间用来写入被调试进程
char *g_pData;

PVOID GetData(HANDLE hProcess, DWORD dwSize)
{
	PVOID pTarget = NULL;

	// 在目标进程申请空间
	pTarget = VirtualAllocEx(hProcess,
		NULL,
		dwSize,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);
	if (pTarget == NULL)
	{
		ShowError("VirtualAllocEx");
		goto exit;
	}

	// 本进程中申请同样大小的内存空间，并初始化为随机数据
	g_pData = (char *)VirtualAlloc(NULL,
		dwSize,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE);

	memset(g_pData, 0, dwSize);

	// 赋值为1-255大小的随机数
	for (DWORD i = 0; i < dwSize; i++)
	{
		g_pData[i] = rand() % 255 + 1;
	}

	// 将本进程中初始化的输入写入到目标进程中
	if (!WriteProcessMemory(hProcess,
		pTarget,
		g_pData,
		dwSize,
		NULL))
	{
		ShowError("WriteProcessMemory");
		goto exit;
	}
exit:
	return pTarget;
}

BOOL FreeData(HANDLE hProcess, PVOID pBaseAddr, DWORD dwSize)
{
	BOOL bRet = TRUE;

	// 释放掉目标进程中申请的内存空间
	if (!VirtualFreeEx(hProcess,
		pBaseAddr,
		dwSize,
		MEM_DECOMMIT))
	{
		ShowError("VirtualFreeEx");
		bRet = FALSE;
		goto exit;
	}

	// 释放掉本进程中申请的内存空间
	if (!VirtualFree(g_pData,
		dwSize,
		MEM_DECOMMIT))
	{
		ShowError("VirtualFreeEx");
		bRet = FALSE;
		g_pData = NULL;
		goto exit;
	}

exit:
	return bRet;
}