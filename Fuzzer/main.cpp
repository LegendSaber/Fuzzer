#include "SRMFuzzer.h"

#define FILE_PATH "vuln.exe"	// 要测试的文件路径
#define FUNC_BEGIN 0x004010B0	// 函数起始地址
#define FUNC_END   0x004010D8   // 函数结束地址

int main()
{
	// 常规模糊测试器
	Fuzzer fuzzer(FILE_PATH);

	fuzzer.BeginFuzzing();

	// 内存模糊测试器
	SRMFuzzer SrmFuzzy(FILE_PATH, FUNC_BEGIN, FUNC_END);

	SrmFuzzy.BeginFuzzing();

	system("pause");
}