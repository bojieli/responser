#include "stdafx.h"
#include "common.h"

void Error(int errorno, const wchar_t *errmsg)
{
	setlocale(LC_ALL, "chs");
	if (errorno == E_FATAL || errorno == E_WARNING) {
		wprintf(errmsg);
		printf("\n");
	}
	if (errorno == E_FATAL)
		return;
}