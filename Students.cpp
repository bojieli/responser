#include "StdAfx.h"
#include "Students.h"
#include "localsto.h"

/* 新建普通学生 */
StuStatic::StuStatic(CString Name, CString StudentId, CString NumericId)
{
	this->Name = Name;
	this->StudentId = StudentId;
	this->NumericId = NumericId;
	this->IsAtClass = false;
}
/* 新建匿名学生 */
StuStatic::StuStatic(CString NumericId)
{
	this->Name = _T("匿名");
	this->StudentId = CString();
	this->NumericId = NumericId;
	this->IsAtClass = false; // 匿名学生是到课的不在名单里的学生
}
/* 新建哨兵 */
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
	++StuNum;
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
/* @brief	根据学号查询学生
 */
StuStatic* StuStaticList::FindByNumericId(CString NumericId)
{
	StuStatic* now = head;
	while((now = now->next)!=NULL)
		if(now->NumericId == NumericId)
			return now;
	return NULL;
}
/* @brief	遍历名单中的学生
 * @param	回调函数，参数依次为学生姓名、学号、数字学号
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

/* @brief	班级
 * @param	sto	本地数据库对象
 * @param	course 课程号
 * @note	点击进入班级时构造此对象
 */
Students::Students(LocalSto* sto, UINT course)
{
	this->head = new Stu(0); //哨兵
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

/* @brief	开始一道新题目的答题
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
/* @brief	接受USB传来的添加答案请求
 * @param	ProductId 答题器ID
 * @param	ANS	添加的答案，为0表示是签到，不为0表示是答题
 * @return	是否操作成功
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
/* @brief	添加正确答案
 * @param	ANS 答案（1字节）
 * @note	如果不在答题时间，则添加的答案是上一题的
 * @return	是否添加成功（如果不在答题时间且数据库访问失败则有可能失败）
 */
bool Students::USBAddCorAnswer(BYTE ANS)
{
	return AddCorAnswer(ANS);
}
bool Students::AddCorAnswer(BYTE ANS)
{
	this->CorAnswer = ANS;
	if (!isStarted) //保存上一题的正确答案
		return Sto->saveCorAnswer(this);
	else
		return true;
}
/* @brief	学生答题
 * @param	ProductID 产品ID
 * @param	ANS	答案（1字节）
 * @param	ansTime 答题时间（从开始答题算起的秒数）
 * @return	是否添加成功
 */
bool Students::AddAnswer(UINT ProductID, BYTE ANS, UINT AnsTime)
{
	Stu *now = FindByProductId(ProductID);
	if (now == NULL || !now->Info->IsAtClass) // 查找失败或未签到，均视为非法请求
		return false;
	if (now->Ans == 0) // 首次回答此题
		++StuAlreadyAns;
	else // 修改此题答案
		--AnswerCount[now->Ans];
	now->AnsTime = AnsTime;
	now->Ans = ANS;
	++AnswerCount[ANS];
	return true;
}
/* @brief	结束答题
 * @return  是否保存成功
 */
bool Students::End(void) 
{
	this->isStarted = false;
	return Sto->saveAnswers(this);
}
/* @brief	学生（答题器）注册并签到
 * @param	NumericId	数字学号
 * @param	ProductId	产品ID
 * @return	是否在静态名单中
 * @note	由于答题器键盘只能输入数字，服务器端维护了数字学号到真实学号和姓名的映射
 */
bool Students::Register(CString NumericId, UINT ProductId)
{
	Sto->setNumericId(NumericId, ProductId); // 保存到数据库
	Stu* now = this->FindByProductId(ProductId);
	if (now == NULL) { // 此答题器尚未注册过
		now = NewStu(ProductId);
	} else { // 此答题器已经注册过，则原来的学生将不在课堂上
		now->Info->IsAtClass = false;
	}
	bool flag = SetInfoByNumericId(now, NumericId);
	SignIn(ProductId);
	return flag;
}
/* @brief	老师给学生正在回答的问题评分
 * @param	ProductId	产品ID
 * @param	mark		评分
 * @return	评分是否成功（只有在答题状态才能评分）
 */
bool Students::TeacherMark(UINT ProductId, BYTE mark)
{
	Stu* now = this->FindByProductId(ProductId);
	if (now == NULL)
		return false;
	now->mark = mark;
	return true;
}
/* @brief	从数据库初始化学号
 * @param	NumericId	数字学号
 * @param	ProductId	产品ID
 * @return	是否在名单中，如果不在名单中则新建一个匿名学生
 * @note	必须在初始化静态信息名单之后调用
 */
bool Students::Add(CString NumericId, UINT ProductId)
{
	Stu* now = new Stu(ProductId); //数据库保证 ProductId 不重复
	AddToList(now);
	bool flag = SetInfoByNumericId(now, NumericId);
	return flag;
}
/* @brief	遍历在线学生
 * @param	回调函数，参数为学生类型对象
 */
void Students::each(void callback(Stu* stu))
{
	Stu* curr = head->next;
	while (curr != NULL) {
		callback(curr);
		curr = curr->next;
	}
}
/* @brief	遍历在线学生
 * @param	回调函数，参数依次为答题器ID、学生姓名、学号、数字学号
 */
void Students::each(void callback(UINT ProductId, CString Name, CString StudentId, CString NumericId))
{
	Stu* curr = head->next;
	while (curr != NULL) {
		callback(curr->ProductId, curr->Info->Name, curr->Info->StudentId, curr->Info->NumericId);
		curr = curr->next;
	}
}
/* @brief	遍历匿名学生
 * @param	回调函数，参数依次为答题器ID、数字学号
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

// 以下是私有函数

/* @brief	答题器签到
 * @return	是否签到成功
 * @note	签到之前必须注册
 */
bool Students::SignIn(UINT ProductId)
{
    Stu* now;
    if (now = FindByProductId(ProductId)) { //在名单中
		if (!now->Info->IsAtClass) {
			now->Info->IsAtClass = true;
			++this->StuAtClass;
			Sto->stuSignIn(ProductId); //保存签到信息到数据库
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
	if (info == NULL) { // 学生不在静态名单中
		now->Info = new StuStatic(NumericId);
		now->isAnonymous = true;
		++AnonymousNum;
		return false;
	} else { // 学生在静态名单中
		now->Info = info;
		if (now->isAnonymous) // 如果以前是匿名，则匿名总数减少了
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