#include "stdafx.h"
#include "common.h"

void Error(int errorno, const wchar_t *errmsg)
{
	if (errorno == E_FATAL || errorno == E_WARNING) {
		TRACE(errmsg);
	}
	if (errorno == E_FATAL) {
		AfxMessageBox(errmsg);
		exit(1);
	}
}