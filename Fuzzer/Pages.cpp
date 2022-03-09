#include "Pages.h"

PMEMORYBLOCKSLIST g_pMemoryBlocksListHead = NULL;

BOOL SavePages(HANDLE hProcess)
{
	BOOL bRet = TRUE, bSaveBlock = TRUE;
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	DWORD dwCursor = 0, dwQuerySize = 0;
	PMEMORYBLOCKSLIST pMemoryBlocksList = NULL;

	while (dwCursor < 0xFFFFFFFF)
	{
		// ��ѯҳ����
		memset(&mbi, 0, sizeof(mbi));
		dwQuerySize = VirtualQueryEx(hProcess,
			(PVOID)dwCursor,
			&mbi,
			sizeof(mbi));
		if (dwQuerySize < sizeof(mbi))
		{
			break;
		}

		bSaveBlock = TRUE;
		// �ж��Ƿ���Ҫ�����ҳ
		if (mbi.State != MEM_COMMIT ||
			mbi.Type == MEM_IMAGE ||
			mbi.Protect & PAGE_READONLY ||
			mbi.Protect & PAGE_EXECUTE_READ ||
			mbi.Protect & PAGE_GUARD ||
			mbi.Protect & PAGE_NOACCESS)
		{
			bSaveBlock = FALSE;
		}

		if (bSaveBlock)
		{
			pMemoryBlocksList = (PMEMORYBLOCKSLIST)VirtualAlloc(NULL,
				sizeof(MEMORYBLOCKSLIST),
				MEM_COMMIT | MEM_RESERVE,
				PAGE_READWRITE);
			if (!pMemoryBlocksList)
			{
				bRet = FALSE;
				ShowError("VirtualAlloc");
				goto exit;
			}

			memset(pMemoryBlocksList, 0, sizeof(MEMORYBLOCKSLIST));
			// ������������ҳ�е�����
			pMemoryBlocksList->read_buf = VirtualAlloc(NULL,
				mbi.RegionSize,
				MEM_COMMIT | MEM_RESERVE,
				PAGE_READWRITE);
			if (!pMemoryBlocksList->read_buf)
			{
				bRet = FALSE;
				ShowError("VirtualAlloc");
				goto exit;
			}

			// ����ҳ�е�����
			if (!ReadProcessMemory(hProcess,
				mbi.BaseAddress,
				pMemoryBlocksList->read_buf,
				mbi.RegionSize,
				NULL))
			{
				bRet = FALSE;
				ShowError("ReadProcessMemory");
				goto exit;
			}

			memcpy(&pMemoryBlocksList->mbi, &mbi, sizeof(mbi));

			// ��������
			pMemoryBlocksList->next = g_pMemoryBlocksListHead;
			g_pMemoryBlocksListHead = pMemoryBlocksList;
		}

		dwCursor += mbi.RegionSize;
	}

exit:
	return bRet;
}

BOOL RestorePages(HANDLE hProcess)
{
	BOOL bRet = TRUE;
	PMEMORYBLOCKSLIST pMemoryBlocksListHead = g_pMemoryBlocksListHead;

	while (pMemoryBlocksListHead)
	{
		if (!WriteProcessMemory(hProcess,
			pMemoryBlocksListHead->mbi.BaseAddress,
			pMemoryBlocksListHead->read_buf,
			pMemoryBlocksListHead->mbi.RegionSize,
			NULL))
		{
			bRet = FALSE;
			ShowError("WriteProcessMemory");
			goto exit;
		}

		// �ָ�ҳ����
		VirtualProtectEx(hProcess,
			pMemoryBlocksListHead->mbi.BaseAddress,
			pMemoryBlocksListHead->mbi.RegionSize,
			pMemoryBlocksListHead->mbi.Protect,
			NULL);

		// ��ȡ��һ��ҳ��Ϣ
		pMemoryBlocksListHead = pMemoryBlocksListHead->next;
	}
exit:
	return bRet;
}