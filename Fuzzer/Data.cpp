#include "Data.h"
#include "auxiliary.h"

// �������������ڴ�ռ�����д�뱻���Խ���
char *g_pData;

PVOID GetData(HANDLE hProcess, DWORD dwSize)
{
	PVOID pTarget = NULL;

	// ��Ŀ���������ռ�
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

	// ������������ͬ����С���ڴ�ռ䣬����ʼ��Ϊ�������
	g_pData = (char *)VirtualAlloc(NULL,
		dwSize,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE);

	memset(g_pData, 0, dwSize);

	// ��ֵΪ1-255��С�������
	for (DWORD i = 0; i < dwSize; i++)
	{
		g_pData[i] = rand() % 255 + 1;
	}

	// ���������г�ʼ��������д�뵽Ŀ�������
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

	// �ͷŵ�Ŀ�������������ڴ�ռ�
	if (!VirtualFreeEx(hProcess,
		pBaseAddr,
		dwSize,
		MEM_DECOMMIT))
	{
		ShowError("VirtualFreeEx");
		bRet = FALSE;
		goto exit;
	}

	// �ͷŵ���������������ڴ�ռ�
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