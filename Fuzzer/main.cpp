#include "SRMFuzzer.h"

#define FILE_PATH "vuln.exe"	// Ҫ���Ե��ļ�·��
#define FUNC_BEGIN 0x004010B0	// ������ʼ��ַ
#define FUNC_END   0x004010D8   // ����������ַ

int main()
{
	// ����ģ��������
	Fuzzer fuzzer(FILE_PATH);

	fuzzer.BeginFuzzing();

	// �ڴ�ģ��������
	SRMFuzzer SrmFuzzy(FILE_PATH, FUNC_BEGIN, FUNC_END);

	SrmFuzzy.BeginFuzzing();

	system("pause");
}