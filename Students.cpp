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
	this->Name = CString(L"����");
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
/* @brief	���ѧ��
 * @param	ID ѧ��
 * @param	ProductID ��ƷID
 * @param	Name ѧ������
 * @note	�ڳ�ʼ��ѧ���б�ʱ���Ż���ô˺���
 */
StuStatic* StuStaticList::Add(CString Name, CString StudentId, CString NumericId)
{
	StuStatic *now = new StuStatic(Name, StudentId, NumericId);
	now->next = head->next;
	head->next = now;
	return now;
}
/* @brief	����ѧ�Ų�ѯѧ��
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

/* @brief	�༶ѧ���б�
 * @param	course �γ̺�
 * @note	�������༶ʱ����˶���
 */
Students::Students(CString course)
{
	this->head = new Stu(0); //�ڱ�
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

/* @brief	��ʼһ������Ŀ�Ĵ���
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
/* @brief	�����ȷ��
 * @param	ANS �𰸣�1�ֽڣ�
 * @note	������ڴ���ʱ�䣬����ӵĴ�����һ���
 * @return	�Ƿ���ӳɹ���������ڴ���ʱ�������ݿ����ʧ�����п���ʧ�ܣ�
 */
bool Students::AddCorAnswer(BYTE ANS)
{
	m_Lock.Lock();
	this->CorAnswer = ANS;
	m_Lock.Unlock();
	if (!isStarted) //������һ�����ȷ��
		return Sto->saveCorAnswer(this);
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
bool Students::AddAnswer(long ProductID, BYTE ANS, unsigned int AnsTime)
{
	Stu *now = FindByProductId(ProductID);
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
		now->AnsTime = AnsTime;
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
	Sto->saveAnswers(this);
	return 0;
}
/* @brief	ѧ������������ǩ��
 * @param	ProductId ��ƷID
 * @return	�Ƿ��������У��������������������ǩ��
 */
bool Students::Register(long ProductId)
{
	Stu* now;
	Sto->stuRegister(ProductId);
	if (now = FindByProductId(ProductId)) { //��������
		now->IsAtClass = true;
		return true;
	} else {
		AddAnonymous(ProductId);
		return false;
	}
}
/* @brief	ѧ����������������ѧ��
 * @param	NumericId	����ѧ��
 * @param	ProductId	��ƷID
 * @return	�Ƿ��������У�����������������½�һ������ѧ��
 * @note	���ڴ���������ֻ���������֣���������ά��������ѧ�ŵ���ʵѧ�ź�������ӳ��
 */
bool Students::SetNumericId(CString NumericId, long ProductId)
{
	Sto->setNumericId(NumericId, ProductId);
	Stu* now = this->FindByProductId(ProductId);
	if (now == NULL) { // �˴�������δǩ��
		now = AddAnonymous(ProductId);
	}
	return SetInfoByNumericId(now, NumericId);
}
/* @brief	�����ݿ��ʼ��ѧ��
 * @param	NumericId	����ѧ��
 * @param	ProductId	��ƷID
 * @return	�Ƿ��������У�����������������½�һ������ѧ��
 * @note	�����ڳ�ʼ����̬��Ϣ����֮�����
 */
bool Students::Add(CString NumericId, long ProductId)
{
	Stu* now = new Stu(ProductId); //���ݿⱣ֤ ProductId ���ظ�
	now->next = head->next;
	head->next = now;
	return SetInfoByNumericId(now, NumericId);
}

// ������˽�к���
bool Students::SetInfoByNumericId(Stu* now, CString NumericId)
{
	StuStatic* info = InfoList.FindByNumericId(NumericId);
	if (info == NULL) { // ѧ�����ھ�̬������
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

CCriticalSection m_Lock;//�߳�ͬ��