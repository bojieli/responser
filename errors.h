#pragma once

#include "stdafx.h"
#include "afx.h"

void Error(int error_level, int errmsg);

// error_level
#define E_FATAL   1 //严重错误，需要关闭程序
#define E_WARNING 2 //警告，需要提示用户
#define E_NOTICE  4 //不干扰主要业务逻辑的错误

// errmsg
#define T_DB_CANNOT_OPEN			0
#define T_DB_CANNOT_INIT			1
#define T_DB_CANNOT_CLOSE			2
#define T_BEGIN_CLASS_BEFORE_END	3
#define T_LAST_CLASS_NOT_ENDED		4
#define T_DB_CANNOT_BEGIN_CLASS		5
#define T_END_CLASS_BEFORE_BEGIN	6
#define T_DB_CANNOT_END_CLASS		7
#define T_DB_SAVE_PROBLEM_FAILED	8
#define T_DB_SAVE_ANSWER_FAILED		9
#define T_DB_SAVE_COURSE_FAILED		10
#define T_DB_UPLOAD_COURSE_FAILED	11
#define T_DB_CANNOT_CLEAN_TABLES	12
#define T_CLOUD_CANNOT_CONNECT		13
#define T_CLOUD_UPLOAD_INTERNAL		14
#define T_CLOUD_RECV_DATA_INVALID	15
#define T_DB_SAVE_FROM_CLOUD_FAILED	16
#define T_CLOUD_REJECT_BY_REMOTE	17

extern CString error_msgs[]; // defined in errors.cpp