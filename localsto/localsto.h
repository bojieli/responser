#pragma once

#include "sqlite3.h"
#include "Students.h"
#include "http.h"

#define SQL_MAXLEN 512

class LocalSto
{
protected:
	sqlite3 *conn = NULL;   //数据库连接
	char *errmsg = NULL;    //操作失败的原因
public:
	LocalSto();
	~LocalSto();
	bool save(Students s);  //保存一次答题信息到本地
	bool uploadToCloud();   //上传答题信息到云端
protected:
	bool syncFromCloud();   //在对象初始化时调用
	bool purgedb();         //在上传到云端后调用
	Http* cloudConn();      //到云端的连接
	bool insert(const char* table, const char* fields, char* data1, ...); // 插入数据
}

void Error(int errorno, CString errmsg);
#define E_FATAL   1 //严重错误，需要关闭程序
#define E_WARNING 2 //警告，需要提示用户
#define E_NOTICE  4 //不干扰主要业务逻辑的错误

LocalSto *dbconn;