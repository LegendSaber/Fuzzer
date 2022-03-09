#include "auxiliary.h"

void ShowError(char *msg)
{
	printf("%s Error %d\n", msg, GetLastError());
}