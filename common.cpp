#include "stdafx.h"
#include "common.h"

void Error(int errorno, CString errmsg)
{
	if (errorno == E_FATAL || errorno == E_WARNING)
		printf("%s\n", errmsg);
	if (errorno == E_FATAL)
		return;
}