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


allStu�ṩ���ⲿ�Ľӿں������£�
���StuStatic m_List,��ֻ��Ҫ����ʦѡ����༶֮����Ӽ��ɣ����ֻ��һ���ӿ�
allStu.m_List.NewStuStatic()

public: //�������
	void Start(); //��ʼ����
	bool End(); //�������⣬�����͵��������򷵻�1�������浽U���򷵻�0

	int USBAddAnswer(BYTE* ProductID, BYTE ANS, UINT ansTime); 
	�ڴ���ģʽ�Ϳ�ǰģʽ��ʹ�ã��ڿ�����ǰģʽʱ���ÿ�ʼ���⣬���ж���Щѧ���Ѿ�����

	void USBAddCorAnswer(BYTE ANS);//�����ȷ�𰸣���ʦ�ʻ��߰����ṩ
public: //ע�����
	bool USBRegister(BYTE* ID,BYTE* ProductID); //ע�ᵽ����


�õ������ݽӿ�
public: //����������
	int QuesTotal;//��Ŀ����,��ʾ����Ҫ������
	int StudTotal;//ѧ����������ʾ����Ҫ������
	int StuAtClass;//��λ��ѧ����������ʾ����Ҫ������
	int StuAlreadyAns;//�Ѿ������ѧ����Ŀ����ʾ����Ҫ������
	bool isStarted;//�Ƿ��ڴ���״̬����ʾ����Ҫ������
	BYTE AnswerCount[64];//��¼ÿһ�ִ𰸵���Ŀ���ܹ�����ѡ��A B C D E F ������ 0x00~0x3f;//��ʾ����Ҫ������

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
	UINT ProductId;			//������ID
	Stu* next;				//ѧ��������һԪ��
public: // ѧ��������Ϣ
	BYTE Ans;				//���һ������Ĵ�
	UINT AnsTime;			//���һ�δ�������ʱ��
	BYTE mark;				//����
	bool IsAtClass;			//�Ƿ��ڿ�����
public: 
	Stu(UINT ProductId);
	~Stu(void);
};

class Students
{
	friend class LocalSto;
public:
	UINT course;			//�༶���
	LocalSto* Sto;			//���ݿ�����
	Stu* head;				//ѧ������head ���ڱ�
	StuStaticList InfoList;	//ѧ����̬��Ϣ����
	UINT beginTime;			//���⿪ʼʱ��
	BYTE CorAnswer;			//���һ����ȷ��
	int QuesTotal;			//��Ŀ����
	int StudTotal;			//ѧ������
	int StuAtClass;			//��λ��ѧ������
	int StuAlreadyAns;		//�Ѿ������ѧ����Ŀ
	bool isStarted;			//�Ƿ��ڴ���״̬
	UINT AnswerCount[64];//��¼ÿһ�ִ𰸵���Ŀ���ܹ�����ѡ��A B C D E F ������ 0x00~0x3f;//��ʾ����Ҫ������
public: //ѡ���༶��ʵ����
	Students(LocalSto* sto, UINT course);
	~Students(void);
public: //������
	void Start(); //��ʼ����
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
	void each(void callback(UINT ProductId, CString Name, CString StudentId));
private:
	bool AddAnswer(UINT ProductId, BYTE ANS, UINT AnsTime); //ѧ������
	bool AddCorAnswer(BYTE ANS); //�����ȷ��
	bool SignIn(UINT ProductId); //������ǩ��
	bool SetInfoByNumericId(Stu* now, CString NumericId);
	Stu* AddAnonymous(UINT ProductId);
	Stu* FindByProductId(UINT ProductId);
};