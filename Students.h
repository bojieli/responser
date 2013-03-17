#pragma once
#include "stdafx.h"
#include "afxmt.h"
#include "localsto.h"
/******************************************�ļ�˵��*****************************************
Students.h �Լ� Students.cpp ��Ҫ���ڴ洢ѧ������
�ṹ�� Answer �洢һ����
�� Stu �洢һ��ѧ���Ļ�����Ϣ�Լ����Ĵ�
�� Students �洢һϵ��ѧ���Լ�����Ľӿڣ�ֻ��һ��ʵ��
���⣬����ͨ�ź���Ҳ��ȫ����������

allStu Ϊ�������̵�ȫ�ֱ���

����ӿ����£�
ÿ�ο�ʼ����ʱ���� allStu.Start(); ��ʼ�����ݿ�
�յ�һ��ѧ���Ĵ���������� allStu.AddAnswer(BYTE* ID,Answer* ANS)

�������ݸ������̣�
1������ʦ����ע��ʱ��ѧ����������ѧ�š�ע��ʱѧ����ѧ�źͲ�ƷID����һ���Ժ�ѧ��ֻ��Ҫ�ò�ƷID���ɡ���ʱ��λ��ֻ����ע��֡��
��ʦ������ѧ����������ѧ�����ص�u�̣������յ���ƷID�����Ƿ��Ѿ�����ѧ�š���ʱ��������¼�����������ͬʱ�յ���ƷID��ѧ�ţ���
ѧ����������һ����������Ҳ����һ����ƷID��ȥ����ѧ�ţ��ʸ񣩡���ʱ���������Ѿ�����һ��ѧ������ӳ���
��ѧ��ע��һ����ƷIDʱ��ȥѰ�����������Ƿ����������֣�����У�������������ѧ���޸��ˡ�
��ѧ��ע��һ����ƷIDʱ�����û�У���ѧ�Ű����µ�ѧ�ţ�������Ϊ������
2������ʦ������ע��ʱ��Ҳ���ǲ�ƷIDû��ע�ᵽ������棬��ô�����Ʒ�ǲ��ܴ����ˡ���Ȼ��վ������ݴ�����λ��������λ�������κδ���
3������һ�ּ�ϯģʽ��Ҳ���ǲ���Ҫ�κ�ע�����ƾ��ID���⡣
ƽʱ����ʦ������ע��ʱ���ǲ������µ�ѧ������ġ�

********************************************�ļ�˵��****************************************/

class StuStatic
{
public:
	CString Name;		//����
	CString StudentId;	//ѧ��
	CString NumericId;	//����ѧ��
	StuStatic* next;
public:
	StuStatic(void);
	StuStatic(CString Name, CString StudentID, CString NumericId);
	~StuStatic(void);
};

class StuStaticList
{
public:
	StuStatic* head;
public: //����ѧ��
	StuStatic* FindByStudentId(CString StudentId);
	StuStatic* FindByNumericId(CString NumericId);
public: //���ѧ��
	StuStatic* Add(CString Name, CString StudentId, CString NumericId);
public:
	StuStaticList(void);
	~StuStaticList(void);
};

class Stu
{
public:
	StuStatic* Info;		//ѧ����̬��Ϣ
	long ProductId;		//������ID
	Stu* next;			//ѧ��������һԪ��
public: // ѧ��������Ϣ
	BYTE Ans;			//���һ������Ĵ�
	unsigned int AnsTime; //���һ�δ�������ʱ��
	bool IsAtClass;		//�Ƿ��ڿ�����
public: 
	Stu(long ProductId);
	~Stu(void);
};

class Students
{
	friend class LocalSto;
public:
	CString course;			//�༶���
	LocalSto* Sto;			//���ݿ�����
	Stu* head;				//ѧ������
	StuStaticList InfoList;	//ѧ����̬��Ϣ����
	unsigned int beginTime;	//��ʼʱ��
	BYTE CorAnswer;			//���һ����ȷ��
	int QuesTotal;			//��Ŀ����
	int StudTotal;			//ѧ������
	int StuAtClass;			//��λ��ѧ������
	int StuAlreadyAns;		//�Ѿ������ѧ����Ŀ
	bool isStarted;			//�Ƿ��ڴ���״̬
	unsigned int AnswerCount[64];//��¼ÿһ�ִ𰸵���Ŀ���ܹ�����ѡ��A B C D E F ������ 0x00~0x3f;//��ʾ����Ҫ������
public: //ѡ���༶��ʵ����
	Students(CString course);
	~Students(void);
public: //������
	void Start(); //��ʼ����
	bool End(); //��������
public: //�������ӿڲ���
	bool AddAnswer(long ProductId, BYTE ANS, unsigned int AnsTime); //ѧ������
	bool AddCorAnswer(BYTE ANS);   //�����ȷ��
	bool Register(long ProductId); //������ǩ��
	bool SetNumericId(CString NumericId, long ProductId); //����������ѧ��
public: //�����ݿ��ʼ��
	bool Add(CString NumericId, long ProductId);
private:
	bool SetInfoByNumericId(Stu* now, CString NumericId);
	Stu* AddAnonymous(long ProductId);
	Stu* FindByProductId(long ProductId);
};

extern Students allStu;
extern CCriticalSection m_Lock;//�߳�ͬ��