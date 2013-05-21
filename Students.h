/******************************************�ļ�˵��*****************************************
Students.h �Լ� Students.cpp ��Ҫ���ڴ洢ѧ������

�������ݸ������̣�
1������ʦ����ע��ʱ��ѧ����������ѧ�š�ע��ʱѧ����ѧ�źͲ�ƷID����һ���Ժ�ѧ��ֻ��Ҫ�ò�ƷID���ɡ���ʱ��λ��ֻ����ע��֡��
��ʦ������ѧ����������ѧ�����ص�u�̣������յ���ƷID�����Ƿ��Ѿ�����ѧ�š���ʱ��������¼�����������ͬʱ�յ���ƷID��ѧ�ţ���
ѧ����������һ����������Ҳ����һ����ƷID��ȥ����ѧ�ţ��ʸ񣩡���ʱ���������Ѿ�����һ��ѧ������ӳ���
��ѧ��ע��һ����ƷIDʱ��ȥѰ�����������Ƿ����������֣�����У�������������ѧ���޸��ˡ�
��ѧ��ע��һ����ƷIDʱ�����û�У���ѧ�Ű����µ�ѧ�ţ�������Ϊ������
2������ʦ������ע��ʱ��Ҳ���ǲ�ƷIDû��ע�ᵽ������棬��ô�����Ʒ�ǲ��ܴ����ˡ���Ȼ��վ������ݴ�����λ��������λ�������κδ���
3������һ�ּ�ϯģʽ��Ҳ���ǲ���Ҫ�κ�ע�����ƾ��ID���⡣
ƽʱ����ʦ������ע��ʱ���ǲ������µ�ѧ������ġ�

API�μ� responser.cpp �еĵ���ʾ����

********************************************�ļ�˵��****************************************/

#pragma once
#include "stdafx.h"
#include "afxmt.h"
#include "localsto.h"

class StuStatic
{
public:
	CString Name;		//����
	CString StudentId;	//ѧ��
	CString NumericId;	//����ѧ��
	int AtClassCount;	//�ڿ����ϵ�ʵ������
	int RefCount;		//����̬�����õļ���
	StuStatic* next;
public:
	StuStatic(CString Name, CString StudentID, CString NumericId); // �½���ͨѧ��
	StuStatic(CString NumericId); // �½�����ѧ��
	StuStatic(void); // �½��ڱ�
	~StuStatic(void);
};

class StuStaticList
{
public:
	StuStaticList(void);
	~StuStaticList(void);
public: //����ѧ��
	StuStatic* FindByStudentId(CString StudentId);
	StuStatic* FindByNumericId(CString NumericId);
public: //���ѧ��
	StuStatic* Add(CString Name, CString StudentId, CString NumericId);
public: //��������
	void each(void callback(StuStatic* s));

public:
	StuStatic* head;
	int StuNum;				//�����е�ѧ������
	int StuAtClass;			//�������ڿ��õ�ѧ������
};

class Stu
{
public:
	StuStatic* Info;		//ѧ����̬��Ϣ
	UINT ProductId;			//������ID
	bool isAnonymous;		//�Ƿ�����
	bool isAtClass;			//�Ƿ��ڿ�����
	Stu* next;				//ѧ��������һԪ��
public: // ѧ��������Ϣ
	BYTE Ans;				//���һ������Ĵ�
	UINT AnsTime;			//���һ�δ�������ʱ��
	BYTE mark;				//����
public: 
	Stu(UINT ProductId);
	~Stu(void);
};

class Students
{
	friend class LocalSto;
public: //ѡ���༶��ʵ����
	Students(LocalSto* sto, UINT course);
	~Students(void);
public: //������
	bool Start(); //��ʼ����
	bool End(); //��������
public: //�������ӿڲ���
	bool USBAddAnswer(UINT ProductId, BYTE ANS); // USB�����ǩ��
	bool USBAddCorAnswer(BYTE ANS); //�����ȷ��
	bool Register(CString NumericId, UINT ProductId); //����������ѧ�ţ�ע��ģʽ
	bool TeacherMark(UINT ProductId, BYTE mark); // ��ʦ��ѧ������
public: //�����ݿ��ʼ��
	bool Add(CString NumericId, UINT ProductId);
public: //����ѧ��
	void each(void callback(Stu* stu));
	void each(void callback(UINT ProductId, CString Name, CString StudentId, CString NumericId, bool isAtClass)); //������ǩ����ѧ��
	void eachAnonymous(void callback(UINT ProductId, CString NumericId, bool isAtClass)); //��������ѧ��
public: // ����ͳ��
	int GetStuTotal(void) {return StudentCount;} // ��̬����ѧ������
	int GetStaticStuTotal(void) {return InfoList.StuNum;} // ��̬����ѧ������
	int GetStuAtClass(void) {return StuAtClass;} // ����ѧ������
	int GetStaticStuAtClass(void) {return InfoList.StuAtClass;} // ��̬���е���ѧ������
	int GetAnonymousNum(void) {return AnonymousNum;} // ����ѧ����
	int GetStuAlreadyAns(void) {return StuAlreadyAns;} // �Ѿ������ѧ����

public:
	UINT course;			//�༶���
	LocalSto* Sto;			//���ݿ�����
	Stu* head;				//ѧ������head ���ڱ�
	StuStaticList InfoList;	//ѧ����̬��Ϣ����
	UINT beginTime;			//���⿪ʼʱ��
	BYTE CorAnswer;			//���һ����ȷ��
	int QuestionNum;		//��Ŀ����
	bool isStarted;			//�Ƿ��ڴ���״̬
	UINT AnswerCount[64];   //��¼ÿһ�ִ𰸵���Ŀ���ܹ�����ѡ��A B C D E F ������ 0x00~0x3f
private:
	int StudentCount;		//��̬���е�ѧ������
	int StuAtClass;			//��λ��ѧ������
	int AnonymousNum;		//����ѧ������
	int StuAlreadyAns;		//�Ѿ������ѧ����Ŀ

private: // ===== ������˽�к��� =====
	void AddToList(Stu* stu);
	bool AddAnswer(UINT ProductId, BYTE ANS, UINT AnsTime); //ѧ������
	bool AddCorAnswer(BYTE ANS); //�����ȷ��
	bool SignIn(UINT ProductId); //������ǩ��
	bool SetInfoByNumericId(Stu* now, CString NumericId);
	Stu* NewStu(UINT ProductId);
	Stu* FindByProductId(UINT ProductId);
};