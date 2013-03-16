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
/* @brief	添加学生
 * @param	ID 学号
 * @param	ProductID 产品ID
 * @param	Name 学生姓名
 * @return	构造出的学生对象
 * @note	在初始化学生列表、匿名答题器加入时，会调用此函数
 */
Stu* Students::Add(BYTE* ID,BYTE* ProductID, CString Name)
{
	Stu *now = new Stu(ID,ProductID,Name);
	now->next = head->next;
	head->next = now;
	StudTotal++;
	return now;
}
/* @brief	查找学生
 * @param	ProductID 答题器产品ID
 * @return	查找到的学生对象，未找到返回 NULL
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
/* @brief	开始一道新题目的答题
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
/* @brief	添加正确答案
 * @param	ANS 答案（1字节）
 * @note	如果不在答题时间，则添加的答案是上一题的
 * @return	是否添加成功（如果不在答题时间且数据库访问失败则有可能失败）
 */
bool Students::USBAddCorAnswer(BYTE ANS)
{
	m_Lock.Lock();
	
		this->CorAnswer.Ans = ANS; // 正确答案无需记录时间，记为0
		this->CorAnswer.ansTime = 0;
	
	m_Lock.Unlock();
	if (!isStarted) //保存上一题的正确答案
		return Sto.saveCorAnswer(this);
	else
		return true;
}
/* @brief	学生答题
 * @param	ProductID 产品ID
 * @param	ANS	答案（1字节）
 * @param	ansTime 答题时间（从开始答题算起的秒数）
 * @return	是否添加成功，如果 ProductID 不在列表中则不成功
 * @note	匿名答题器在调用此函数答题前，必须通过 USBRegister 签到
 */
bool Students::USBAddAnswer(BYTE* ProductID, BYTE ANS, unsigned int ansTime)
{
	Stu *now = Find(ProductID);
	if(now!=NULL)
	{
		if(now->Ans==0) // 首次回答此题
		{
			m_Lock.Lock();
	
				StuAlreadyAns++;
			
			m_Lock.Unlock();
		}
		if(!now->IsAtClass) // 如果还没签到，先签到
		{
			m_Lock.Lock();
			
				StuAtClass++;
				now->IsAtClass = true;

			m_Lock.Unlock();
		}
		if(now->Ans!=ANS) // 修改此题答案
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
/* @brief	结束答题
 * @return  若发送到服务器则返回1，若保存到U盘则返回0
 */
bool Students::End(void) 
{
	this->isStarted = false;
	Sto.saveAnswers(this);
	return 0;
}
/* @brief	学生（答题器）签到
 * @param	ID 学号
 * @param	ProductID 产品ID
 * @return	是否在名单中，如果不在名单中则匿名签到
 */
bool Students::USBRegister(BYTE* ID,BYTE* ProductID)
{
	Stu* now;
	StuStatic* ListTemp;
	ListTemp = m_List.FindStu(ID);
	if((now = Find(ProductID))==NULL)
	{
		if(ListTemp==NULL)
			now = Add(ID,ProductID,_T("匿名"));
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
			now->Name = _T("匿名");
		else
			now->Name = ListTemp->Name;
		return true;
	}
}


Students allStu;
CCriticalSection m_Lock;//线程同步