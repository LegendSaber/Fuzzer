#pragma once
#include "Fuzzer.h"
#include "Data.h"

class SRMFuzzer : public Fuzzer
{
public:
	SRMFuzzer(char *szFilePath,
			  DWORD dwFuncBegin,
			  DWORD dwFuncEnd);
	void BeginFuzzing();

private:
	DWORD dwFuncBegin;		// ������ʼ��ַ
	DWORD dwFuncEnd;		// ����������ַ
	BYTE bOrgFuncBegin;		// ������ʼһ�ֽ�
	BYTE bOrgFuncEnd;		// ��������һ�ֽ�
	BYTE bInt3;				// ����int 3���ֽ�
};
