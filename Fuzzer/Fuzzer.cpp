#include "Fuzzer.h"
#include "auxiliary.h"


Fuzzer::Fuzzer(char *szFilePath)
{
	if (strlen(szFilePath) < MAX_PATH)
	{
		strcpy(this->szFilePath, szFilePath);
	}
	else
	{
		printf("文件名过长\n");
	}
}

char *Fuzzer::GetFilePath()
{
	return this->szFilePath;
}

void Fuzzer::BeginFuzzing()
{
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	DEBUG_EVENT DbgEvt = { 0 };				// 保存调试事件的数据结构
	bool bContinue = true;					// 是否继续
	DWORD dwContinueStatus = DBG_CONTINUE;	// 恢复继续执行用的状态代码
	DWORD dwSize = 4;						// 输入数据的大小
	char szInput[MAXBYTE] = { 0 };			// 输入数据缓冲区
	bool bExit = false;						// 被调试进程结束则设为true

	printf("Fuzzy Begin....\n");

	while (dwSize < MAXBYTE)
	{
		// 随机输入数据
		memset(szInput, 0, MAXBYTE);
		for (DWORD i = 0; i < dwSize; i++)
		{
			szInput[i] = rand() % 255 + 1;
		}

		memset(&si, 0, sizeof(si));
		memset(&pi, 0, sizeof(pi));
		si.cb = sizeof(si);
		if (!CreateProcess(this->szFilePath,
			szInput,
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE |
			DEBUG_PROCESS |
			DEBUG_ONLY_THIS_PROCESS,
			NULL,
			NULL,
			&si,
			&pi))
		{
			ShowError("CreateProcess");
			goto exit;
		}

		bExit = false;
		while (bContinue && !bExit)
		{
			memset(&DbgEvt, 0, sizeof(DbgEvt));

			// 等待调试事件发生
			bContinue = WaitForDebugEvent(&DbgEvt, INFINITE);
			if (!bContinue)
			{
				ShowError("WaitForDebugEvent");
				break;
			}

			switch (DbgEvt.dwDebugEventCode)
			{
			case EXCEPTION_DEBUG_EVENT:
			{
				switch (DbgEvt.u.Exception.ExceptionRecord.ExceptionCode)
				{
					// 捕获到程序异常
				case EXCEPTION_ACCESS_VIOLATION:
				{
					printf(".....Catch Access Violation.....\n");
					printf("dwSize is: %d\n", dwSize);
					printf("Address is 0x%X\n", DbgEvt.u.Exception.ExceptionRecord.ExceptionAddress);
					bExit = true;
					break;
				}
				default:
				{
					break;
				}
				}
				break;
			}
			case EXIT_PROCESS_DEBUG_EVENT:
			{
				bExit = true;	// 该进程已退出
				break;
			}
			default:
			{
				break;
			}
			}

			// 恢复被调试进程继续运行
			bContinue = ContinueDebugEvent(DbgEvt.dwProcessId,
				DbgEvt.dwThreadId,
				dwContinueStatus);
			if (!bContinue)
			{
				ShowError("ContinueDebugEvent");
				break;
			}
		}

		// 增加数据大小，供下一次进行测试
		dwSize += 4;

		if (pi.hProcess) CloseHandle(pi.hProcess);
		if (pi.hThread) CloseHandle(pi.hThread);
	}

exit:
	printf("Fuzzy End...\n");
}
