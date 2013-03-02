#pragma once

#include "stdafx.h"
#include "sqlite3.h"
#include "Students.h"
#include "http.h"

#define SQL_MAXLEN 512

class LocalSto
{
protected:
	sqlite3 *dbconn;   //���ݿ�����
	char *errmsg;      //����ʧ�ܵ�ԭ��
	int lectureID;     //��ǰ�ǵڼ��ڿΣ����ڴ洢
public:
	LocalSto();
	~LocalSto();
	bool save(Students* s);     //����һ�δ�����Ϣ������
	bool stuRegister(Stu* stu); //ѧ��ע��
	bool uploadToCloud();       //�ϴ�������Ϣ���ƶ�
	bool syncFromCloud();       //���ƶ�ͬ��ѧ������
	bool initStuNames(StuStaticList* m_List); //��ʼ��ѧ���������ݽṹ
protected:
	CString rowsToStr(const char* sql); // ����ѯ������л�����
protected:
	bool insert(const char* table, const char* fields, char* data1, ...); // �����ַ�������
	bool insert(const char* table, const char* fields, long data1, ...); // ������������
	bool getAll(const char* table, int (*callback)(void*,int,char**,char**)); // ��ȡ���е��������ݣ�ÿ���ص�
	bool select(const char* table, const char* field, char* value, int (*callback)(void*,int,char**,char**));
	bool update(const char* table, const char* searchField, char* searchValue, const char* updateField, char* updateValue);
	bool query(const char* sql); // ԭ�����ݿ��ѯ����Ҫ����ֵ
	bool squery(const char* pattern, ...); // query��sprintf�Ľ��
	bool query(const char* sql, int (*callback)(void*,int,char**,char**), void* argtocallback); // ԭ�����ݿ��ѯ
};
