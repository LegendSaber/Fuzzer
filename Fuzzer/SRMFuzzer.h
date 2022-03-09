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
	DWORD dwFuncBegin;		// 函数起始地址
	DWORD dwFuncEnd;		// 函数结束地址
	BYTE bOrgFuncBegin;		// 函数起始一字节
	BYTE bOrgFuncEnd;		// 函数结束一字节
	BYTE bInt3;				// 保存int 3的字节
};
