/* ���������ش洢
 * �÷���
 * 1. ������ʱ��Ҫָ����վID�Ͱ�ȫ��ʶ��
 * 2. getCourses ��ȡ���༶��Ϣ
 * 3. �û�ѡ��༶��setCourseId ���õ�ǰ�༶ID
 * 4. ���� initStuNames ��ʼ���༶ѧ���б�
 * 5. ��ѧ��ǩ��ʱ
 *    ��ÿ�������ʱ���� saveAnswers
 *    ����ʦ������ȷ��ʱ���� saveCorAnswer
 */

#pragma once
#include "stdafx.h"
#include "sqlite3.h"
#include "Students.h"
#include "cloudconn.h"
#include "baseStation.h"
#include "courses.h"

#define SQL_MAXLEN 512

class LocalSto
{
	friend class Students;
protected:
	sqlite3 *dbconn;		//���ݿ�����
	char *errmsg;			//����ʧ�ܵ�ԭ��
	UINT course;			//�γ̺�
	UINT lectureID;			//��ǰ�ǵڼ��ڿΣ����ڴ洢
private:
	UINT StationID;		    //��վID
	CString StationToken;	//��վ��ȫ��ʶ��
public:
	LocalSto(UINT StationID, CString StationToken);
	~LocalSto(void);
	bool getCourses(Courses* c);			  //��ʼ���γ���Ϣ���ݽṹ
	bool setCourseId(UINT course_id);		  //���õ�ǰ�γ�ID�����ú���ܱ��������Ϣ
public:
	bool saveAnswers(Students* s);            //����һ����Ĵ�����Ϣ
	bool saveCorAnswer(Students* s);		  //������ȷ��
	bool setNumericId(CString NumericId, UINT ProductId); //����ѧ��
	bool stuSignIn(UINT ProductId);			  //ѧ��ǩ��
	bool initStuNames(Students* s);			  //��ʼ��ѧ���������ݽṹ
private:
	CString rowsToStr(const char* sql); // ����ѯ������л�����
	bool initDbFile();			//��ʼ�����ݿ��ļ�
	bool addCourse(CString name, CString info);	//��ӿγ�
	bool uploadToCloud();       //�ϴ�������Ϣ���ƶˣ�save ʱ���Զ�����
	bool syncFromCloud();       //���ƶ�ͬ��ѧ��������ʵ����ʱ���Զ�����
	char* loadDataInStr(const char* table, const char* columns, const int column_count, char* str); // load data in file
private:
	bool insert(const char* table, const char* fields, char* data1, ...); // �����ַ�������
	bool insert(const char* table, const char* fields, UINT data1, ...); // ������������
	bool getAll(const char* table, int (*callback)(void*,int,char**,char**)); // ��ȡ���е��������ݣ�ÿ���ص�
	bool select(const char* table, const char* field, char* value, int (*callback)(void*,int,char**,char**));
	bool update(const char* table, const char* searchField, char* searchValue, const char* updateField, char* updateValue);
	bool query(const char* sql); // ԭ�����ݿ��ѯ����Ҫ����ֵ
	bool squery(const char* pattern, ...); // query��sprintf�Ľ��
	bool query(const char* sql, int (*callback)(void*,int,char**,char**), void* argtocallback); // ԭ�����ݿ��ѯ
	char* selectFirst(const char* pattern, ...); // ���ַ�����ʽ���ز�ѯ�ĵ�һ�����
};