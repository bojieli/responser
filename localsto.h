#pragma once

#include "stdafx.h"
#include "sqlite3.h"
#include "Students.h"
#include "http.h"
#include "baseStation.h"

#define SQL_MAXLEN 512

class LocalSto
{
	friend class Students;
protected:
	sqlite3 *dbconn;   //���ݿ�����
	char *errmsg;      //����ʧ�ܵ�ԭ��
	char *course;	   //�γ̺ţ��Ѿ�ת�壩
	int lectureID;     //��ǰ�ǵڼ��ڿΣ����ڴ洢
public:
	LocalSto(char* course_id);
	~LocalSto(void);
	bool saveAnswers(Students* s);            //����һ����Ĵ�����Ϣ
	bool saveCorAnswer(Students* s);		  //������ȷ��
	bool setNumericId(CString NumericId, long ProductId); //����ѧ��
	bool stuRegister(long ProductId);		  //ѧ��ǩ��
	bool initStuNames(Students* s);			  //��ʼ��ѧ���������ݽṹ
private:
	CString rowsToStr(const char* sql); // ����ѯ������л�����
	bool initDbFile();			//��ʼ�����ݿ��ļ�
	bool uploadToCloud();       //�ϴ�������Ϣ���ƶˣ�save ʱ���Զ�����
	bool syncFromCloud();       //���ƶ�ͬ��ѧ��������ʵ����ʱ���Զ�����
	char* loadDataInStr(const char* table, const char* columns, const int column_count, char* str); // load data in file
private:
	bool insert(const char* table, const char* fields, char* data1, ...); // �����ַ�������
	bool insert(const char* table, const char* fields, long data1, ...); // ������������
	bool getAll(const char* table, int (*callback)(void*,int,char**,char**)); // ��ȡ���е��������ݣ�ÿ���ص�
	bool select(const char* table, const char* field, char* value, int (*callback)(void*,int,char**,char**));
	bool update(const char* table, const char* searchField, char* searchValue, const char* updateField, char* updateValue);
	bool query(const char* sql); // ԭ�����ݿ��ѯ����Ҫ����ֵ
	bool squery(const char* pattern, ...); // query��sprintf�Ľ��
	bool query(const char* sql, int (*callback)(void*,int,char**,char**), void* argtocallback); // ԭ�����ݿ��ѯ
	char* selectFirst(const char* pattern, ...); // ���ַ�����ʽ���ز�ѯ�ĵ�һ�����
};