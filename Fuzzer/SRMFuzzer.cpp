#include "SRMFuzzer.h"
#include "Thread.h"
#include "Pages.h"

SRMFuzzer::SRMFuzzer(char *szFilePath,
	DWORD dwFuncBegin,
	DWORD dwFuncEnd) : Fuzzer(szFilePath)
{
	this->dwFuncBegin = dwFuncBegin;
	this->dwFuncEnd = dwFuncEnd;
	this->bOrgFuncBegin = 0;
	this->bOrgFuncEnd = 0;
	this->bInt3 = 0xCC;
}

void SRMFuzzer::BeginFuzzing()
{
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	DEBUG_EVENT DbgEvt = { 0 };				// ��������¼������ݽṹ
	bool bContinue = true;					// �Ƿ����
	DWORD dwContinueStatus = DBG_CONTINUE;	// �ָ�����ִ���õ�״̬����
	DWORD dwSize = 4;						// Ҫ�޸ĵ����ݴ�С
	PVOID pInputAddr = NULL;				// �������Խ�������������ݵ�ַ
	CONTEXT cxt = { 0 };
	bool bFirstPointer = true;				// ��¼�Ƿ��ǵ�һ�δ���������ʼ��ַ������ϵ�

	si.cb = sizeof(si);
	if (!CreateProcess(GetFilePath(),
		NULL,
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

	printf("Fuzzy Begin...\n");
	while (bContinue && dwSize < MAX_PATH)
	{
		memset(&DbgEvt, 0, sizeof(DbgEvt));

		// �ȴ������¼�����
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
				// �����쳣����ӡ����
			case EXCEPTION_ACCESS_VIOLATION:
			{
				printf(".....Catch Access Violation...\n");
				printf("Address: 0x%X\n", DbgEvt.u.Exception.ExceptionRecord.ExceptionAddress);
				printf("dwSize:%d\n", dwSize);
				goto exit;
			}
			case EXCEPTION_BREAKPOINT:
			{
				// �����ϵ����жϴ���int 3�ϵ��λ��
				DWORD dwPointAddr = (DWORD)DbgEvt.u.Exception.ExceptionRecord.ExceptionAddress;
				if (dwPointAddr == this->dwFuncBegin)
				{
					// �ϵ�λ���Ǻ�����ʼ��ַ

					// �Ƿ��ǵ�һ�δ���������ʼ��������ϵ�
					if (bFirstPointer)
					{
						// �������̵߳�״̬
						cxt.ContextFlags = CONTEXT_FULL;
						if (!GetThreadContext(pi.hThread, &cxt))
						{
							ShowError("GetThreadContext");
							goto exit;
						}

						// ��EIP����ָ��������ʼ��ַ
						cxt.Eip -= sizeof(this->bInt3);

						// ��һ�εı�����Ϊ�����̼߳��뵽ȫ�ֱ����е�ʱ��
						// EIP����ָ������ʼ��ַ
						if (!SetThreadContext(pi.hThread, &cxt))
						{
							ShowError("SetThreadContext");
							goto exit;
						}

						// �����߳�CONTEXT���ڴ�״̬��ȫ�ֱ�����
						if (!SaveThreadContext(pi.dwProcessId) ||
							!SavePages(pi.hProcess))
						{
							goto exit;
						}

						bFirstPointer = false;
					}

					// �ָ�������ʼ���ֽ�
					if (!WriteProcessMemory(pi.hProcess,
						(PVOID)this->dwFuncBegin,
						&(this->bOrgFuncBegin),
						sizeof(this->bOrgFuncBegin),
						NULL))
					{
						ShowError("WriteProcessMemory");
						goto exit;
					}

					// ����ĩβ����Ϊ����ϵ�
					if (!WriteProcessMemory(pi.hProcess,
						(PVOID)this->dwFuncEnd,
						&(this->bInt3),
						sizeof(this->bInt3),
						NULL))
					{
						ShowError("WriteProcessMemory");
						goto exit;
					}

					// �ڱ����Խ��������벢��ʼ����������
					pInputAddr = GetData(pi.hProcess, dwSize);
					if (!pInputAddr)
					{
						goto exit;
					}

					// �����������ݵ�ַΪ������������ݵ�ַ
					if (!WriteProcessMemory(pi.hProcess,
						(PVOID)(cxt.Esp + 4),
						(PVOID)&pInputAddr,
						sizeof(pInputAddr),
						NULL))
					{
						ShowError("WriteProcessMemory");
						goto exit;
					}

					// ���ǵ�һ�δ����ϵ㣬��ʱ��Ҫ�޸����߳�CONTEXT��EIP
					if (!bFirstPointer)
					{
						if (!SetThreadContext(pi.hThread, &cxt))
						{
							ShowError("SetThreadContext");
							goto exit;
						}
					}
				}
				else if (dwPointAddr == this->dwFuncEnd)
				{
					// ִ�е�����ĩβ

					// �ͷŵ�������ڴ�
					if (!FreeData(pi.hProcess, pInputAddr, dwSize))
					{
						goto exit;
					}

					// �����趨������ڴ��С
					pInputAddr = NULL;
					dwSize += 4;

					// �ָ��߳�CONTEXT���ڴ�״̬
					if (!RestoreThreadContext() ||
						!RestorePages(pi.hProcess))
					{
						goto exit;
					}

					// �޸ĺ�����ʼ��ַ�е��ֽ�Ϊ����ϵ�
					if (!WriteProcessMemory(pi.hProcess,
						(PVOID)this->dwFuncBegin,
						&(this->bInt3),
						sizeof(this->bInt3),
						NULL))
					{
						ShowError("WriteProcessMemory");
						goto exit;
					}
				}
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}
		case CREATE_PROCESS_DEBUG_EVENT:
		{
			// �����Խ��̴�����ʱ�򽫺�����ԭʼ�ֽڶ�ȡ����
			// ���д������ϵ�
			if (DbgEvt.dwProcessId == pi.dwProcessId)
			{
				if (!ReadProcessMemory(pi.hProcess,
					(PVOID)this->dwFuncBegin,
					&(this->bOrgFuncBegin),
					sizeof(this->bOrgFuncBegin),
					NULL))
				{
					ShowError("ReadProcessMemory");
					goto exit;
				}

				if (!WriteProcessMemory(pi.hProcess,
					(PVOID)this->dwFuncBegin,
					&(this->bInt3),
					sizeof(this->bInt3),
					NULL))
				{
					ShowError("WriteProcessMemory");
					goto exit;
				}

				if (!ReadProcessMemory(pi.hProcess,
					(PVOID)this->dwFuncEnd,
					&(this->bOrgFuncEnd),
					sizeof(this->bOrgFuncEnd),
					NULL))
				{
					ShowError("ReadProcessMemory");
					goto exit;
				}

				if (!WriteProcessMemory(pi.hProcess,
					(PVOID)this->dwFuncEnd,
					&(this->bInt3),
					sizeof(this->bInt3),
					NULL))
				{
					ShowError("WriteProcessMemory");
					goto exit;
				}

				printf("Init BeakPoint ok...\n");
			}
			break;
		}
		default:
		{
			break;
		}
		}

		// �ָ������Խ��̼�������
		bContinue = ContinueDebugEvent(DbgEvt.dwProcessId,
			DbgEvt.dwThreadId,
			dwContinueStatus);;
		if (!bContinue)
		{
			ShowError("ContinueDebugEvent");
			break;
		}
	}


exit:
	printf("Fuzzy End...\n");
}
