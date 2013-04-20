#include "stdafx.h"
#include "common.h"

void Error(int errorno, const wchar_t *errmsg)
{
	if (errorno == E_FATAL || errorno == E_WARNING) {
		AfxMessageBox(errmsg);
	}
	if (errorno == E_FATAL) {
		exit(1);
	}
}