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
		// 查询页属性
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
		// 判断是否是要保存的页
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
			// 申请用来保存页中的数据
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

			// 保存页中的数据
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

			// 连入链表
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

		// 恢复页属性
		VirtualProtectEx(hProcess,
			pMemoryBlocksListHead->mbi.BaseAddress,
			pMemoryBlocksListHead->mbi.RegionSize,
			pMemoryBlocksListHead->mbi.Protect,
			NULL);

		// 获取下一个页信息
		pMemoryBlocksListHead = pMemoryBlocksListHead->next;
	}
exit:
	return bRet;
}