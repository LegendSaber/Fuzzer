#pragma once
#include <cstdio>
#include <Windows.h>
#include "auxiliary.h"

class Fuzzer
{
public:
	Fuzzer(char *szFilePath);
	char *GetFilePath();		// 获取启动文件路径
	void BeginFuzzing();			// 以常规方式开启测试
private:
	char szFilePath[MAX_PATH];	// 保存启动文件路径
};
