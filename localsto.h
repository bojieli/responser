/* ���������ش洢
 * �÷���
 * 1. ������ʱ��Ҫָ����վID�Ͱ�ȫ��ʶ��
 * 2. getCourses ��ȡ���༶��Ϣ
 * 3. �û�ѡ��༶��setCurCourse ���õ�ǰ�༶ID
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
	int error;				//�������
	char *errmsg;			//����ʧ�ܵ�ԭ��
	UINT course;			//�γ�ID�����ÿγ�ID����ܿ�ʼһ�ڿ�
	UINT lectureID;			//��ǰ�ǵڼ��ڿΣ����ڴ洢
private:
	UINT StationID;		    //��վID
	CString StationToken;	//��վ��ȫ��ʶ��
public:
	LocalSto(UINT StationID, CString StationToken);
	~LocalSto(void);
public:
	bool getCourses(Courses* c);			  //��ʼ���γ���Ϣ���ݽṹ
	bool beginCourse(UINT courseID);	      //��ʼһ�ڿ�
	bool endCourse();						  //������ڿ�
	bool addCourse(Course* c);				  //��ӿγ̲���ȡ�γ�ID
public:
	bool saveAnswers(Students* s);            //����һ����Ĵ�����Ϣ
	bool saveCorAnswer(Students* s);		  //������ȷ��
	bool setNumericId(CString NumericId, UINT ProductId); //����ѧ��
	bool stuSignIn(UINT ProductId);			  //ѧ��ǩ��
	bool initStuStaticList(Students* s);	  //��ʼ��ѧ����̬��
	bool initStudents(Students* s);			  //��ʼ��ѧ����̬��

private:
	CString rowsToStr(CString sql); // ����ѯ������л�����
	CString rowsToStrIfNotChanged(CString sql); // ��δ���ϴ����Ĳ�ѯ������л�����
	bool initDbFile();			//��ʼ�����ݿ��ļ�

	bool uploadToCloud();       //�ϴ�������Ϣ���ƶˣ�save ʱ���Զ�����
	bool syncFromCloud();       //���ƶ�ͬ��ѧ��������ʵ����ʱ���Զ�����
	CString loadDataInStr(CString table, CString columns, const int column_count, CString str); // load data in file
private:
	bool insert(CString table, CString fields, CString data1, ...); // �����ַ�������
	bool insert(CString table, CString fields, UINT data1, ...); // ������������
	bool getAll(CString table, int (*callback)(void*,int,char**,char**)); // ��ȡ���е��������ݣ�ÿ���ص�
	bool select(CString table, CString field, CString value, int (*callback)(void*,int,char**,char**));
	bool update(CString table, CString searchField, CString searchValue, CString updateField, CString updateValue);
	bool query(CString sql); // ԭ�����ݿ��ѯ����Ҫ����ֵ
	bool squery(CString pattern, ...); // query �� Format �Ľ��
	bool query(CString sql, int (*callback)(void*,int,char**,char**), void* argtocallback); // ԭ�����ݿ��ѯ
	CString selectFirst(CString pattern, ...); // ���ַ�����ʽ���ز�ѯ�ĵ�һ�����
};