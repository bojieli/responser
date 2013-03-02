#pragma once

#include "stdafx.h"
#include "sqlite3.h"
#include "Students.h"
#include "http.h"

#define SQL_MAXLEN 512

class LocalSto
{
protected:
	sqlite3 *dbconn;   //数据库连接
	char *errmsg;      //操作失败的原因
	int lectureID;     //当前是第几节课，用于存储
public:
	LocalSto();
	~LocalSto();
	bool save(Students* s);     //保存一次答题信息到本地
	bool stuRegister(Stu* stu); //学生注册
	bool uploadToCloud();       //上传答题信息到云端
	bool syncFromCloud();       //从云端同步学生姓名
	bool initStuNames(StuStaticList* m_List); //初始化学生姓名数据结构
protected:
	CString rowsToStr(const char* sql); // 将查询结果序列化出来
protected:
	bool insert(const char* table, const char* fields, char* data1, ...); // 插入字符串数据
	bool insert(const char* table, const char* fields, long data1, ...); // 插入整型数据
	bool getAll(const char* table, int (*callback)(void*,int,char**,char**)); // 获取表中的所有数据，每条回调
	bool select(const char* table, const char* field, char* value, int (*callback)(void*,int,char**,char**));
	bool update(const char* table, const char* searchField, char* searchValue, const char* updateField, char* updateValue);
	bool query(const char* sql); // 原生数据库查询，不要返回值
	bool squery(const char* pattern, ...); // query与sprintf的结合
	bool query(const char* sql, int (*callback)(void*,int,char**,char**), void* argtocallback); // 原生数据库查询
};
