#pragma once
#include <cstdio>
#include <Windows.h>
#include "auxiliary.h"

class Fuzzer
{
public:
	Fuzzer(char *szFilePath);
	char *GetFilePath();		// ��ȡ�����ļ�·��
	void BeginFuzzing();			// �Գ��淽ʽ��������
private:
	char szFilePath[MAX_PATH];	// ���������ļ�·��
};
