#pragma once

#include "stdafx.h"
#include "afx.h"

void Error(int errorno, const wchar_t *errmsg);
#define E_FATAL   1 //���ش�����Ҫ�رճ���
#define E_WARNING 2 //���棬��Ҫ��ʾ�û�
#define E_NOTICE  4 //��������Ҫҵ���߼��Ĵ���
