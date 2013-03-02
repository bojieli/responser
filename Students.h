#pragma once

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
ÿ�ν�������ʱ���� allStu.End(); ���浽U�̻��ƶ�

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

#define ID_LENGTH 5
#define PID_LENGTH 4

class StuStatic
{
public:
	StuStatic(CString Name1,BYTE* ID1)
	{
		Name = Name1;
		for(int i=0;i<ID_LENGTH;i++)
			ID[i] = ID1[i];
		next = NULL;
	}	
	StuStatic()
	{
	}
	~StuStatic(void)
	{
		
	}
public:
	CString Name;
	BYTE ID[5];
	StuStatic* next;
	bool isMe(BYTE* ID1)//����ID�Ƿ�һ��,���ڲ���
	{
		for(int i = 0; i<ID_LENGTH; i++)
		if (ID1[i] != this->ID[i])
			return false;
			return true;
	}
};
class StuStaticList
{
public:
	StuStaticList()
	{
		StaticList = new StuStatic();
	}
	~StuStaticList()
	{
		StuStatic* temp1 = StaticList;
		StuStatic* temp2 = temp1->next;
		while(temp1!=NULL)
		{
			delete temp1;
			temp1 = temp2;
			temp2 = temp1->next;
		}
	}
public:
	StuStatic* StaticList;
	void NewStuStatic(CString Name,BYTE* ID)
	{
		StuStatic* now = new StuStatic(Name,ID);
		now->next = StaticList->next;
		StaticList->next = now;
	}
	StuStatic* FindStu(BYTE* ID1)
	{
		StuStatic* temp = StaticList;
		while((temp = temp->next)!=NULL)
		{
			if(temp->isMe(ID1))
				return temp;
		}
		return 0;
	}
};

class Stu
{
public: 
	Stu(void);
	Stu(BYTE* ID1,BYTE* ProductID1,CString Name1);
	~Stu(void);
	Stu* next; //ѧ��������һԪ��
	
public: //һ��ѧ���Ļ�����Ϣ
	CString ID[5];//ѧ��ѧ���ַ���
	BYTE ProductID[4];//ѧ��ע��ʱ�Ĳ�ƷID Product ID
	CString Name;//ѧ��������
	BYTE Ans;//��AnsΪ0ʱ��ʾû������
	unsigned int ansTime;//��������Ҫ��ʱ��
	bool isMyProduct(BYTE* ProductID1);//����ProductID�Ƿ�һ�������ڲ���
};

class Students
{
public:
	Students(void);
	~Students(void);
public:
	Stu* head; //ѧ������
	StuStaticList m_List; // ѧ��������ӳ���
	friend class LocalSto;
	LocalSto* localSto; // ���ݿ�����
public:
	Stu* Add(BYTE* ID,BYTE* ProductID, CString Name);//���ѧ��
	Stu* Find(BYTE* ProductID);
	Stu CorAnswer;//�洢��ȷ��
public: //����������
	int QuesTotal;//��Ŀ����
	int StudTotal;//ѧ������
	unsigned long beginTime;//��ʼ����ʱ��
	bool isStarted;//�Ƿ��ڴ���״̬
public: //�������
	void Start(); //��ʼ����
	bool End(); //�������⣬�����͵��������򷵻�1�������浽U���򷵻�0
	int USBAddAnswer(BYTE* ProductID, BYTE ANS, unsigned int ansTime); //��Ӵ𰸣����� 1 ��ʾԭ�У����� 0 ��ʾ���������� -1 ��ʾ���ڴ���ʱ��
	void USBAddCorAnswer(BYTE ANS);//�����ȷ��
public: //ע�����
	bool USBRegister(BYTE* ID,BYTE* ProductID); //ע�ᵽ����
};

extern Students allStu;
