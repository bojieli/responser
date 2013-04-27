#include "StdAfx.h"
#include "Students.h"
#include "localsto.h"

/* �½���ͨѧ�� */
StuStatic::StuStatic(CString Name, CString StudentId, CString NumericId)
{
	this->Name = Name;
	this->StudentId = StudentId;
	this->NumericId = NumericId;
	this->IsAtClass = false;
}
/* �½�����ѧ�� */
StuStatic::StuStatic(CString NumericId)
{
	this->Name = _T("����");
	this->StudentId = CString();
	this->NumericId = NumericId;
	this->IsAtClass = false; // ����ѧ���ǵ��εĲ����������ѧ��
}
/* �½��ڱ� */
StuStatic::StuStatic(void)
{
}
StuStatic::~StuStatic(void)
{
}

StuStaticList::StuStaticList(void)
{
	head = new StuStatic();
	StuNum = 0;
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
	++StuNum;
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
/* @brief	����ѧ�Ų�ѯѧ��
 */
StuStatic* StuStaticList::FindByNumericId(CString NumericId)
{
	StuStatic* now = head;
	while((now = now->next)!=NULL)
		if(now->NumericId == NumericId)
			return now;
	return NULL;
}
/* @brief	���������е�ѧ��
 * @param	�ص���������������Ϊѧ��������ѧ�š�����ѧ��
 */
void StuStaticList::each(void callback(CString Name, CString StudentId, CString NumericId, bool IsAtClass))
{
	StuStatic* curr = head->next;
	while (curr != NULL) {
		callback(curr->Name, curr->StudentId, curr->NumericId, curr->IsAtClass);
		curr = curr->next;
	}
}

Stu::Stu(UINT ProductId)
{
	Info = NULL;
	this->ProductId = ProductId;
	isAnonymous = false;
	next = NULL;
	Ans = 0;
	AnsTime = 0;
	mark = 0;
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
	this->QuestionNum = 0;
	this->OnlineStuNum = 0;
	this->AnonymousNum = 0;
	this->course = course;
	this->isStarted = false;
	this->StuAtClass = 0;
	this->StuAlreadyAns = 0;
	this->Sto = sto;
	for(int i = 0;i<64;i++)
		this->AnswerCount[i] = 0;
	sto->beginCourse(course);
	sto->initStuStaticList(this);
	sto->initStudents(this);
}
Students::~Students(void)
{
	this->Sto->endCourse();
	while (this->head != NULL)
	{
		Stu *temp = this->head;
		this->head = temp->next;
		delete temp;
	}
}

/* @brief	��ʼһ������Ŀ�Ĵ���
 */
bool Students::Start()
{
	isStarted = true;
	StuAlreadyAns = 0;
	QuestionNum++;
	for(int i = 0;i<64;i++)
		AnswerCount[i] = 0;
	Stu* curr = head->next;
	while (curr != NULL) {
		curr->Ans = 0;
		curr->AnsTime = 0;
		curr->mark = 0;
		curr = curr->next;
	}
	return true;
}
/* @brief	����USB��������Ӵ�����
 * @param	ProductId ������ID
 * @param	ANS	��ӵĴ𰸣�Ϊ0��ʾ��ǩ������Ϊ0��ʾ�Ǵ���
 * @return	�Ƿ�����ɹ�
 */
bool Students::USBAddAnswer(UINT ProductId, BYTE ANS)
{
	bool flag = SignIn(ProductId);
	if (flag && ANS != 0) {
		if (this->isStarted)
			return AddAnswer(ProductId, ANS, (UINT)time(NULL) - beginTime);
		else
			return false;
	}
	return flag;
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
 * @return	�Ƿ���ӳɹ�
 */
bool Students::AddAnswer(UINT ProductID, BYTE ANS, UINT AnsTime)
{
	Stu *now = FindByProductId(ProductID);
	if (now == NULL || !now->Info->IsAtClass) // ����ʧ�ܻ�δǩ��������Ϊ�Ƿ�����
		return false;
	if (now->Ans == 0) // �״λش����
		++StuAlreadyAns;
	else // �޸Ĵ����
		--AnswerCount[now->Ans];
	now->AnsTime = AnsTime;
	now->Ans = ANS;
	++AnswerCount[ANS];
	return true;
}
/* @brief	��������
 * @return  �Ƿ񱣴�ɹ�
 */
bool Students::End(void) 
{
	this->isStarted = false;
	return Sto->saveAnswers(this);
}
/* @brief	ѧ������������ע�Ტǩ��
 * @param	NumericId	����ѧ��
 * @param	ProductId	��ƷID
 * @return	�Ƿ��ھ�̬������
 * @note	���ڴ���������ֻ���������֣���������ά��������ѧ�ŵ���ʵѧ�ź�������ӳ��
 */
bool Students::Register(CString NumericId, UINT ProductId)
{
	Sto->setNumericId(NumericId, ProductId); // ���浽���ݿ�
	Stu* now = this->FindByProductId(ProductId);
	if (now == NULL) { // �˴�������δע���
		now = NewStu(ProductId);
	} else { // �˴������Ѿ�ע�������ԭ����ѧ�������ڿ�����
		now->Info->IsAtClass = false;
	}
	bool flag = SetInfoByNumericId(now, NumericId);
	SignIn(ProductId);
	return flag;
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
	AddToList(now);
	bool flag = SetInfoByNumericId(now, NumericId);
	return flag;
}
/* @brief	��������ѧ��
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
/* @brief	��������ѧ��
 * @param	�ص���������������Ϊ������ID��ѧ��������ѧ�š�����ѧ��
 */
void Students::each(void callback(UINT ProductId, CString Name, CString StudentId, CString NumericId))
{
	Stu* curr = head->next;
	while (curr != NULL) {
		callback(curr->ProductId, curr->Info->Name, curr->Info->StudentId, curr->Info->NumericId);
		curr = curr->next;
	}
}
/* @brief	��������ѧ��
 * @param	�ص���������������Ϊ������ID������ѧ��
 */
void Students::eachAnonymous(void callback(UINT ProductId, CString NumericId))
{
	Stu* curr = head->next;
	while (curr != NULL) {
		if (curr->isAnonymous)
			callback(curr->ProductId, curr->Info->NumericId);
		curr = curr->next;
	}
}

// ������˽�к���

/* @brief	������ǩ��
 * @return	�Ƿ�ǩ���ɹ�
 * @note	ǩ��֮ǰ����ע��
 */
bool Students::SignIn(UINT ProductId)
{
    Stu* now;
    if (now = FindByProductId(ProductId)) { //��������
		if (!now->Info->IsAtClass) {
			now->Info->IsAtClass = true;
			++this->StuAtClass;
			Sto->stuSignIn(ProductId); //����ǩ����Ϣ�����ݿ�
		}
        return true;
    }
	return false;
}
void Students::AddToList(Stu* now) {
	now->next = head->next;
	head->next = now;
	++OnlineStuNum;
}
bool Students::SetInfoByNumericId(Stu* now, CString NumericId)
{
	StuStatic* info = InfoList.FindByNumericId(NumericId);
	if (info == NULL) { // ѧ�����ھ�̬������
		now->Info = new StuStatic(NumericId);
		now->isAnonymous = true;
		++AnonymousNum;
		return false;
	} else { // ѧ���ھ�̬������
		now->Info = info;
		if (now->isAnonymous) // �����ǰ������������������������
			--AnonymousNum;
		now->isAnonymous = false;
		return true;
	}
}
Stu* Students::NewStu(UINT ProductId)
{
	Stu* now = new Stu(ProductId);
	AddToList(now);
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