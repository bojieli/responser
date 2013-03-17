#include "StdAfx.h"
#include "Students.h"
#include "localsto.h"

StuStatic::StuStatic(CString Name, CString StudentId, CString NumericId)
{
	this->Name = Name;
	this->StudentId = StudentId;
	this->NumericId = NumericId;
}
StuStatic::StuStatic(void)
{
	this->Name = CString(L"匿名");
	this->StudentId = CString();
	this->NumericId = CString();
}
StuStatic::~StuStatic(void)
{
}

StuStaticList::StuStaticList(void)
{
	head = new StuStatic();
}
StuStaticList::~StuStaticList(void)
{
	while (head != NULL)
	{
		StuStatic *temp = head;
		head = temp->next;
		delete temp;
	}
}
/* @brief	添加学生
 * @param	ID 学号
 * @param	ProductID 产品ID
 * @param	Name 学生姓名
 * @note	在初始化学生列表时，才会调用此函数
 */
StuStatic* StuStaticList::Add(CString Name, CString StudentId, CString NumericId)
{
	StuStatic *now = new StuStatic(Name, StudentId, NumericId);
	now->next = head->next;
	head->next = now;
	return now;
}
/* @brief	根据学号查询学生
 */
StuStatic* StuStaticList::FindByStudentId(CString StudentId)
{
	StuStatic* now = head;
	while((now = now->next)!=NULL)
		if(now->StudentId == StudentId)
			return now;
	return NULL;
}
StuStatic* StuStaticList::FindByNumericId(CString NumericId)
{
	StuStatic* now = head;
	while((now = now->next)!=NULL)
		if(now->NumericId == NumericId)
			return now;
	return NULL;
}

Stu::Stu(long ProductId)
{
	Ans = 0;
	AnsTime = 0;
	next = NULL;
	IsAtClass = false;
	this->ProductId = ProductId;
	Info = NULL;
}
Stu::~Stu(void)
{	
}

/* @brief	班级学生列表
 * @param	course 课程号
 * @note	点击进入班级时构造此对象
 */
Students::Students(CString course)
{
	this->head = new Stu(0); //哨兵
	this->QuesTotal = 0;
	this->StudTotal = 0;
	this->course = course;
	this->isStarted = false;
	this->StuAtClass = 0;
	this->StuAlreadyAns = 0;
	this->Sto = new LocalSto((LPSTR)(LPCTSTR)course);
	Sto->initStuNames(this);
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

/* @brief	开始一道新题目的答题
 */
void Students::Start()
{
	m_Lock.Lock();
	isStarted = true;
	StuAlreadyAns = 0;
	QuesTotal++;
	for(int i = 0;i<64;i++)
		AnswerCount[i] = 0;
	m_Lock.Unlock();
}
/* @brief	添加正确答案
 * @param	ANS 答案（1字节）
 * @note	如果不在答题时间，则添加的答案是上一题的
 * @return	是否添加成功（如果不在答题时间且数据库访问失败则有可能失败）
 */
bool Students::AddCorAnswer(BYTE ANS)
{
	m_Lock.Lock();
	this->CorAnswer = ANS;
	m_Lock.Unlock();
	if (!isStarted) //保存上一题的正确答案
		return Sto->saveCorAnswer(this);
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
bool Students::AddAnswer(long ProductID, BYTE ANS, unsigned int AnsTime)
{
	Stu *now = FindByProductId(ProductID);
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
		now->AnsTime = AnsTime;
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
	Sto->saveAnswers(this);
	return 0;
}
/* @brief	学生（答题器）签到
 * @param	ProductId 产品ID
 * @return	是否在名单中，如果不在名单中则匿名签到
 */
bool Students::Register(long ProductId)
{
	Stu* now;
	Sto->stuRegister(ProductId);
	if (now = FindByProductId(ProductId)) { //在名单中
		now->IsAtClass = true;
		return true;
	} else {
		AddAnonymous(ProductId);
		return false;
	}
}
/* @brief	学生（答题器）设置学号
 * @param	NumericId	数字学号
 * @param	ProductId	产品ID
 * @return	是否在名单中，如果不在名单中则新建一个匿名学生
 * @note	由于答题器键盘只能输入数字，服务器端维护了数字学号到真实学号和姓名的映射
 */
bool Students::SetNumericId(CString NumericId, long ProductId)
{
	Sto->setNumericId(NumericId, ProductId);
	Stu* now = this->FindByProductId(ProductId);
	if (now == NULL) { // 此答题器尚未签到
		now = AddAnonymous(ProductId);
	}
	return SetInfoByNumericId(now, NumericId);
}
/* @brief	从数据库初始化学号
 * @param	NumericId	数字学号
 * @param	ProductId	产品ID
 * @return	是否在名单中，如果不在名单中则新建一个匿名学生
 * @note	必须在初始化静态信息名单之后调用
 */
bool Students::Add(CString NumericId, long ProductId)
{
	Stu* now = new Stu(ProductId); //数据库保证 ProductId 不重复
	now->next = head->next;
	head->next = now;
	return SetInfoByNumericId(now, NumericId);
}

// 以下是私有函数
bool Students::SetInfoByNumericId(Stu* now, CString NumericId)
{
	StuStatic* info = InfoList.FindByNumericId(NumericId);
	if (info == NULL) { // 学生不在静态名单中
		now->Info = new StuStatic();
		return false;
	} else {
		now->Info = info;
		return true;
	}
}
Stu* Students::AddAnonymous(long ProductId)
{
	Stu* now = new Stu(ProductId);
	now->IsAtClass = true;
	now->next = head->next;
	head->next = now;
	return now;
}
Stu* Students::FindByProductId(long ProductId)
{
	Stu* now = head;
	while((now = now->next)!=NULL)
		if(now->ProductId == ProductId)
			return now;
	return NULL;
}

CCriticalSection m_Lock;//线程同步