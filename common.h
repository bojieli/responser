#pragma once

#include "stdafx.h"
#include "afx.h"

void Error(int errorno, CString errmsg);
#define E_FATAL   1 //���ش�����Ҫ�رճ���
#define E_WARNING 2 //���棬��Ҫ��ʾ�û�
#define E_NOTICE  4 //��������Ҫҵ���߼��Ĵ���

void Error(int errorno, CString errmsg)
{
	if (errorno == E_FATAL || errorno == E_WARNING)
		printf("%s\n", errmsg);
	if (errorno == E_FATAL)
		return;
}