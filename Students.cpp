#include "StdAfx.h"
#include "Students.h"
#include "localsto.h"

Students allStu;
StuStaticList stuNames;

Stu::Stu(void)
{
	Ans = 0;
	ansTime = 0;
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
	this->isStarted =false;
	this->localSto = new LocalSto;
	this->localSto->initStuNames(&(this->m_List));
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
	this->beginTime = (unsigned long)time(NULL);
	Stu* temp = head;
	while((temp = temp->next)!=NULL)
	{
		temp->Ans = 0;
		temp->ansTime = 0;
	}
}
void Students::USBAddCorAnswer(BYTE ANS)
{
	this->CorAnswer.Ans = ANS; // 正确答案无需记录时间，记为0
	this->CorAnswer.ansTime = 0;
}
int Students::USBAddAnswer(BYTE* ProductID, BYTE ANS, unsigned int ansTime)
{
	if (ANS == 0)

	if (!this->isStarted)
		return -1;
	Stu *now = Find(ProductID);
	if(now!=NULL)
	{
		now->Ans = ANS;
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
	localSto->save(this);
	return 0;
}
bool Students::USBRegister(BYTE* newID,BYTE* newProductID) //返回 1 表示原有，返回 0 表示新增
{
	Stu* now;
	StuStatic* ListTemp;
	ListTemp = m_List.FindStu(newID);
	if((now = Find(newProductID)) == NULL) // 新增
	{
		if(ListTemp==NULL)
			Add(newID, newProductID, _T("匿名"));
		else
			Add(newID, newProductID, ListTemp->Name);
		return false;
	}
	else // 已有
	{
		for(int i =0;i< ID_LENGTH;i++)
			now->ID[i] = newID[i]; // 学号
		if(ListTemp==NULL)
			now->Name = _T("匿名");
		else
			now->Name = ListTemp->Name;
		return true;
	}
}