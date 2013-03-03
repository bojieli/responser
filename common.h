#pragma once

#include "stdafx.h"
#include "afx.h"

void Error(int errorno, CString errmsg);
#define E_FATAL   1 //严重错误，需要关闭程序
#define E_WARNING 2 //警告，需要提示用户
#define E_NOTICE  4 //不干扰主要业务逻辑的错误
