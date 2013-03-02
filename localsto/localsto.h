#pragma once

#include "sqlite3.h"
#include "Students.h"
#include "http.h"

#define SQL_MAXLEN 512

class LocalSto
{
protected:
	sqlite3 *conn = NULL;   //���ݿ�����
	char *errmsg = NULL;    //����ʧ�ܵ�ԭ��
public:
	LocalSto();
	~LocalSto();
	bool save(Students s);  //����һ�δ�����Ϣ������
	bool uploadToCloud();   //�ϴ�������Ϣ���ƶ�
protected:
	bool syncFromCloud();   //�ڶ����ʼ��ʱ����
	bool purgedb();         //���ϴ����ƶ˺����
	Http* cloudConn();      //���ƶ˵�����
	bool insert(const char* table, const char* fields, char* data1, ...); // ��������
}

void Error(int errorno, CString errmsg);
#define E_FATAL   1 //���ش�����Ҫ�رճ���
#define E_WARNING 2 //���棬��Ҫ��ʾ�û�
#define E_NOTICE  4 //��������Ҫҵ���߼��Ĵ���

LocalSto *dbconn;