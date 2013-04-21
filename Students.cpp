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
	this->Name = _T("����");
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

Stu::Stu(UINT ProductId)
{
	Ans = 0;
	AnsTime = 0;
	next = NULL;
	mark = 0;
	IsAtClass = false;
	this->ProductId = ProductId;
	Info = NULL;
}
Stu::~Stu(void)
{
}

/* @brief	�༶
 * @param	sto	�������ݿ����
 * @param	course �γ̺�
 * @note	�������༶ʱ����˶���
 */
Students::Students(LocalSto* sto, UINT course)
{
	this->head = new Stu(0); //�ڱ�
	this->QuesTotal = 0;
	this->StudTotal = 0;
	this->course = course;
	this->isStarted = false;
	this->StuAtClass = 0;
	this->StuAlreadyAns = 0;
	this->Sto = sto;
	sto->setCurCourse(course);
	sto->initStuNames(this);
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
	isStarted = true;
	StuAlreadyAns = 0;
	QuesTotal++;
	for(int i = 0;i<64;i++)
		AnswerCount[i] = 0;
}
/* @brief	����USB��������Ӵ�����
 * @param	ProductId ������ID
 * @param	ANS	��ӵĴ𰸣�Ϊ0��ʾ��ǩ������Ϊ0��ʾ�Ǵ���
 * @return	�Ƿ�����ɹ�
 */
bool Students::USBAddAnswer(UINT ProductId, BYTE ANS)
{
	if (ANS == 0)
		return SignIn(ProductId);
	else
		return AddAnswer(ProductId, ANS, (UINT)time(NULL) - beginTime);
}
/* @brief	�����ȷ��
 * @param	ANS �𰸣�1�ֽڣ�
 * @note	������ڴ���ʱ�䣬����ӵĴ�����һ���
 * @return	�Ƿ���ӳɹ���������ڴ���ʱ�������ݿ����ʧ�����п���ʧ�ܣ�
 */
bool Students::USBAddCorAnswer(BYTE ANS)
{
	return AddCorAnswer(ANS);
}
bool Students::AddCorAnswer(BYTE ANS)
{
	this->CorAnswer = ANS;
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
bool Students::AddAnswer(UINT ProductID, BYTE ANS, UINT AnsTime)
{
	Stu *now = FindByProductId(ProductID);
	if(now!=NULL)
	{
		if(now->Ans==0) // �״λش����
		{
			StuAlreadyAns++;
		}
		if(!now->IsAtClass) // �����ûǩ������ǩ��
		{
			StuAtClass++;
			now->IsAtClass = true;
		}
		if(now->Ans!=ANS) // �޸Ĵ����
		{
			AnswerCount[now->Ans]--;
			AnswerCount[ANS]++;
			now->Ans = ANS;
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
bool Students::SignIn(UINT ProductId)
{
	Stu* now;
	Sto->stuSignIn(ProductId);
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
bool Students::Register(CString NumericId, UINT ProductId)
{
	Sto->setNumericId(NumericId, ProductId);
	Stu* now = this->FindByProductId(ProductId);
	if (now == NULL) { // �˴�������δǩ��
		now = AddAnonymous(ProductId);
	}
	return SetInfoByNumericId(now, NumericId);
}
/* @brief	��ʦ��ѧ�����ڻش����������
 * @param	ProductId	��ƷID
 * @param	mark		����
 * @return	�����Ƿ�ɹ���ֻ���ڴ���״̬�������֣�
 */
bool Students::TeacherMark(UINT ProductId, BYTE mark)
{
	Stu* now = this->FindByProductId(ProductId);
	if (now == NULL)
		return false;
	now->mark = mark;
	return true;
}
/* @brief	�����ݿ��ʼ��ѧ��
 * @param	NumericId	����ѧ��
 * @param	ProductId	��ƷID
 * @return	�Ƿ��������У�����������������½�һ������ѧ��
 * @note	�����ڳ�ʼ����̬��Ϣ����֮�����
 */
bool Students::Add(CString NumericId, UINT ProductId)
{
	Stu* now = new Stu(ProductId); //���ݿⱣ֤ ProductId ���ظ�
	now->next = head->next;
	head->next = now;
	return SetInfoByNumericId(now, NumericId);
}
/* @brief	�����༶������ѧ��
 * @param	�ص�����������Ϊѧ�����Ͷ���
 */
void Students::each(void callback(Stu* stu))
{
	Stu* curr = head->next;
	while (curr != NULL) {
		callback(curr);
		curr = curr->next;
	}
}
/* @brief	�����༶������ѧ��
 * @param	�ص���������������Ϊ������ID��ѧ��������ѧ��
 */
void Students::each(void callback(UINT ProductId, CString Name, CString StudentId))
{
	Stu* curr = head->next;
	while (curr != NULL) {
		callback(curr->ProductId, curr->Info->Name, curr->Info->StudentId);
		curr = curr->next;
	}
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
Stu* Students::AddAnonymous(UINT ProductId)
{
	Stu* now = new Stu(ProductId);
	now->IsAtClass = true;
	now->next = head->next;
	head->next = now;
	return now;
}
Stu* Students::FindByProductId(UINT ProductId)
{
	Stu* now = head;
	while((now = now->next)!=NULL)
		if(now->ProductId == ProductId)
			return now;
	return NULL;
}