#include "StdAfx.h"
#include "Students.h"
#include "localsto.h"

Stu::Stu(void)
{
	Ans = 0;
	ansTime = 0;
	next = NULL;
	IsAtClass = false;
}
Stu::Stu(BYTE* ID1,BYTE* ProductID1,CString Name1)
{
	for(int i=0;i<ID_LENGTH;i++)
		ID[i] = ID1[i];
	for(int i=0;i<PID_LENGTH;i++)
		ProductID[i] = ProductID1[i];
	Name = Name1;
	Ans = 0;
	ansTime = 0;
	next = NULL;
	IsAtClass = false;
}
Stu::~Stu(void)
{
	
}
bool Stu::isMyProduct(BYTE* ProductID1)
{
	for(int i = 0; i<PID_LENGTH; i++)
		if (ProductID1[i] != ProductID[i])
			return false;
	return true;
}


Students::Students(void)
{
	this->head = new Stu;
	this->head->next = NULL;
	this->QuesTotal = 0;
	this->StudTotal = 0;
	isStarted =false;
	StuAtClass = 0;
	Sto.initStuNames(&this->m_List);
}
Students::~Students(void)
{
	while (this->head != NULL)
	{
		Stu *temp = this->head;
		this->head = temp->next;
		delete temp;
	}
}
/* @brief	���ѧ��
 * @param	ID ѧ��
 * @param	ProductID ��ƷID
 * @param	Name ѧ������
 * @return	�������ѧ������
 * @note	�ڳ�ʼ��ѧ���б���������������ʱ������ô˺���
 */
Stu* Students::Add(BYTE* ID,BYTE* ProductID, CString Name)
{
	Stu *now = new Stu(ID,ProductID,Name);
	now->next = head->next;
	head->next = now;
	StudTotal++;
	return now;
}
/* @brief	����ѧ��
 * @param	ProductID ��������ƷID
 * @return	���ҵ���ѧ������δ�ҵ����� NULL
 */
Stu* Students::Find(BYTE* ProductID)
{
	Stu* now = head;
	while((now = now->next)!=NULL)
	{
		if(now->isMyProduct(ProductID))
			return now;
	}
	return NULL;
}
/* @brief	��ʼһ������Ŀ�Ĵ���
 */
void Students::Start()
{
	this->isStarted = true;
	this->QuesTotal++;
	Stu* temp = head;
	m_Lock.Lock();
	
		for(int i = 0;i<64;i++)
			AnswerCount[i] = 0;
	
	m_Lock.Unlock();
	while((temp = temp->next)!=NULL)
	{
		m_Lock.Lock();
	
			AnswerCount[0]++;

		m_Lock.Unlock();
		temp->Ans = 0;
		temp->ansTime = 0;
	}
}
/* @brief	�����ȷ��
 * @param	ANS �𰸣�1�ֽڣ�
 * @note	������ڴ���ʱ�䣬����ӵĴ�����һ���
 * @return	�Ƿ���ӳɹ���������ڴ���ʱ�������ݿ����ʧ�����п���ʧ�ܣ�
 */
bool Students::USBAddCorAnswer(BYTE ANS)
{
	m_Lock.Lock();
	
		this->CorAnswer.Ans = ANS; // ��ȷ�������¼ʱ�䣬��Ϊ0
		this->CorAnswer.ansTime = 0;
	
	m_Lock.Unlock();
	if (!isStarted) //������һ�����ȷ��
		return Sto.saveCorAnswer(this);
	else
		return true;
}
/* @brief	ѧ������
 * @param	ProductID ��ƷID
 * @param	ANS	�𰸣�1�ֽڣ�
 * @param	ansTime ����ʱ�䣨�ӿ�ʼ���������������
 * @return	�Ƿ���ӳɹ������ ProductID �����б����򲻳ɹ�
 * @note	�����������ڵ��ô˺�������ǰ������ͨ�� USBRegister ǩ��
 */
bool Students::USBAddAnswer(BYTE* ProductID, BYTE ANS, unsigned int ansTime)
{
	Stu *now = Find(ProductID);
	if(now!=NULL)
	{
		if(now->Ans==0) // �״λش����
		{
			m_Lock.Lock();
	
				StuAlreadyAns++;
			
			m_Lock.Unlock();
		}
		if(!now->IsAtClass) // �����ûǩ������ǩ��
		{
			m_Lock.Lock();
			
				StuAtClass++;
				now->IsAtClass = true;

			m_Lock.Unlock();
		}
		if(now->Ans!=ANS) // �޸Ĵ����
		{
			m_Lock.Lock();
	
				AnswerCount[now->Ans]--;
				AnswerCount[ANS]++;
				now->Ans = ANS;
			
			m_Lock.Unlock();		
		}
		now->ansTime = ansTime;
		return true;
	}
	else return false;
}
/* @brief	��������
 * @return  �����͵��������򷵻�1�������浽U���򷵻�0
 */
bool Students::End(void) 
{
	this->isStarted = false;
	Sto.saveAnswers(this);
	return 0;
}
/* @brief	ѧ������������ǩ��
 * @param	ID ѧ��
 * @param	ProductID ��ƷID
 * @return	�Ƿ��������У��������������������ǩ��
 */
bool Students::USBRegister(BYTE* ID,BYTE* ProductID)
{
	Stu* now;
	StuStatic* ListTemp;
	ListTemp = m_List.FindStu(ID);
	if((now = Find(ProductID))==NULL)
	{
		if(ListTemp==NULL)
			now = Add(ID,ProductID,_T("����"));
		else
			now = Add(ID,ProductID,ListTemp->Name);
		Sto.stuRegister(now);
		return false;
	}
	else
	{
		for(int i =0;i< ID_LENGTH;i++)
			now->ID[i] = ID[i];
		if(ListTemp==NULL)
			now->Name = _T("����");
		else
			now->Name = ListTemp->Name;
		return true;
	}
}


Students allStu;
CCriticalSection m_Lock;//�߳�ͬ��