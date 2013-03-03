#include "StdAfx.h"
#include "Students.h"

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
Stu* Students::Add(BYTE* ID,BYTE* ProductID, CString Name)
{
	Stu *now = new Stu(ID,ProductID,Name);
	now->next = head->next;
	head->next = now;
	StudTotal++;
	return now;
}
Stu* Students::Find(BYTE* ProductID)
{
	Stu* now = head;
	while((now = now->next)!=NULL)
	{
		if(now->isMyProduct(ProductID))
			return now;
	}
	return 0;
}
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
void Students::USBAddCorAnswer(BYTE ANS)
{
	m_Lock.Lock();
	
		this->CorAnswer.Ans = ANS; // 正确答案无需记录时间，记为0
		this->CorAnswer.ansTime = 0;
	
	m_Lock.Unlock();
}
int Students::USBAddAnswer(BYTE* ProductID, BYTE ANS, unsigned int ansTime)
{
	//if (!this->isStarted)
//		return -1;
	Stu *now = Find(ProductID);
	if(now!=NULL)
	{
		
		if(now->Ans==0)
		{
			m_Lock.Lock();
	
				StuAlreadyAns++;
			
			m_Lock.Unlock();
		}
		if(!now->IsAtClass)
		{
			m_Lock.Lock();
			
				StuAtClass++;

			m_Lock.Unlock();
			now->IsAtClass = true;
		}
		if(now->Ans!=ANS)
		{
			m_Lock.Lock();
	
				AnswerCount[now->Ans]--;
				AnswerCount[ANS]++;
			
			m_Lock.Unlock();
			now->Ans = ANS;
		}
		now->ansTime = ansTime;	
		
		return 1;
	}
	else
	{
		return 0;
	}
}
bool Students::End(void) // 若发送到服务器则返回1，若保存到U盘则返回0
{
	this->isStarted = false;
	return 0;
}
bool Students::USBRegister(BYTE* ID,BYTE* ProductID) //返回 1 表示原有，返回 0 表示新增
{
	Stu* now;
	StuStatic* ListTemp;
	ListTemp = m_List.FindStu(ID);
	if((now = Find(ProductID))==NULL)
	{
		if(ListTemp==NULL)
		{
			Add(ID,ProductID,_T("匿名"));
		}
		else
		{
			Add(ID,ProductID,ListTemp->Name);
		}
		return false;
	}
	else
	{
		for(int i =0;i< ID_LENGTH;i++)
			now->ID[i] = ID[i];
		if(ListTemp==NULL)
		{
			now->Name = _T("匿名");
		}
		else
			now->Name = ListTemp->Name;
		return true;
	}
}


Students allStu;
CCriticalSection m_Lock;//线程同步