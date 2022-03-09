#include "Thread.h"
#include "auxiliary.h"

PCONTEXTLIST g_pCtxListHead = NULL;			// ͷ�ڵ㣬�����������е��߳�

// �����߳�CONTEXT
BOOL SaveThreadContext(DWORD dwPid)
{
	BOOL bRet = TRUE, bContninue = TRUE;
	HANDLE hSnap = NULL, hThread = NULL;
	THREADENTRY32 te = { 0 };
	PCONTEXTLIST pContextList = NULL;
	CONTEXT cxt;

	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	if (!hSnap)
	{
		ShowError("CreateToolhelp32Snapshot");
		bRet = FALSE;
		goto exit;
	}

	te.dwSize = sizeof(te);
	bContninue = Thread32First(hSnap, &te);
	while (bContninue)
	{
		// ���ݽ���PIDѡ��Ҫ������߳�
		if (te.th32OwnerProcessID == dwPid)
		{
			// ����ռ����������߳���Ϣ
			pContextList = (PCONTEXTLIST)VirtualAlloc(NULL,
				sizeof(CONTEXTLIST),
				MEM_RESERVE | MEM_COMMIT,
				PAGE_READWRITE);
			if (!pContextList)
			{
				bRet = FALSE;
				ShowError("VirtualAlloc");
				goto exit;
			}

			memset(pContextList, 0, sizeof(CONTEXTLIST));

			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
			if (!hThread)
			{
				bRet = FALSE;
				ShowError("OpenThread");
				goto exit;
			}

			if (SuspendThread(hThread) == 0xFFFFFFFF)
			{
				bRet = FALSE;
				ShowError("SuspendThread");
				goto exit;
			}

			// �����߳�ID
			pContextList->dwThreadId = te.th32ThreadID;

			// ��ȡ�̵߳�Context
			pContextList->ctx.ContextFlags = CONTEXT_FULL;
			if (!GetThreadContext(hThread, &(pContextList->ctx)))
			{
				bRet = FALSE;
				ShowError("GetThreadContext");
				goto exit;
			}

			// ��Context����pCtxListHead��
			pContextList->next = g_pCtxListHead;
			g_pCtxListHead = pContextList;

			if (ResumeThread(hThread) == 0xFFFFFFFF)
			{
				bRet = FALSE;
				ShowError("SuspendThread");
				goto exit;
			}

			CloseHandle(hThread);
		}
		bContninue = Thread32Next(hSnap, &te);
	}
exit:
	return bRet;
}

// �ָ��߳�CONTEXT
BOOL RestoreThreadContext()
{
	BOOL bRet = TRUE;
	HANDLE hThread = NULL;
	PCONTEXTLIST pContextList = NULL;

	pContextList = g_pCtxListHead;
	while (pContextList)
	{
		hThread = OpenThread(THREAD_ALL_ACCESS,
			FALSE,
			pContextList->dwThreadId);
		if (!hThread)
		{
			bRet = FALSE;
			ShowError("OpenThread");
			goto exit;
		}

		if (SuspendThread(hThread) == 0xFFFFFFFF)
		{
			bRet = FALSE;
			ShowError("SuspendThread");
			goto exit;
		}

		if (!SetThreadContext(hThread, &(pContextList->ctx)))
		{
			bRet = FALSE;
			ShowError("SetThreadContext");
			goto exit;
		}

		if (ResumeThread(hThread) == 0xFFFFFFFF)
		{
			bRet = FALSE;
			ShowError("SuspendThread");
			goto exit;
		}
		CloseHandle(hThread);
		pContextList = pContextList->next;
	}

exit:
	return bRet;
}