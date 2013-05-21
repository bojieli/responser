#include "StdAfx.h"
#include "Students.h"
#include "localsto.h"

/* 新建普通学生 */
StuStatic::StuStatic(CString Name, CString StudentId, CString NumericId)
{
	this->Name = Name;
	this->StudentId = StudentId;
	this->NumericId = NumericId;
	this->AtClassCount = 0;
	this->RefCount = 0;
	this->next = NULL;
}
/* 新建匿名学生 */
StuStatic::StuStatic(CString NumericId)
{
	this->Name = _T("匿名");
	this->StudentId = CString();
	this->NumericId = NumericId;
	this->AtClassCount = 0;
	this->RefCount = 0;
	this->next = NULL;
}
/* 新建哨兵 */
StuStatic::StuStatic(void)
{
	this->Name = CString();
	this->StudentId = CString();
	this->NumericId = CString();
	this->AtClassCount = 0;
	this->RefCount = 0;
	this->next = NULL;
}
StuStatic::~StuStatic(void)
{
}

StuStaticList::StuStaticList(void)
{
	head = new StuStatic();
	StuNum = 0;
	StuAtClass = 0;
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
 * @param	回调函数，参数为静态学生
 */
void StuStaticList::each(void callback(StuStatic* s))
{
	StuStatic* curr = head->next;
	while (curr != NULL) {
		callback(curr);
		curr = curr->next;
	}
}

Stu::Stu(UINT ProductId)
{
	Info = NULL;
	this->ProductId = ProductId;
	isAnonymous = true; // 如果没找到对应的 Info，当然是匿名
	isAtClass = false;
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
	this->StudentCount = 0;
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
	SignIn(ProductID); // 如果没签到，先签到
	Stu *now = FindByProductId(ProductID);
	if (now == NULL) // 查找失败，均视为非法请求
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
	Sto->setNumericId(NumericId, ProductId); // 保存到数据库（数据库会自动保证 ProductId 的唯一性）
	Stu* now = this->FindByProductId(ProductId);
	if (now == NULL) { // 此答题器尚未注册过
		now = NewStu(ProductId);
	} else { // 此答题器已经注册过，则取消注册
		if (now->isAtClass) { // 如果名单中此人已经到课，则取消之
			if (0 == --now->Info->AtClassCount)
				--InfoList.StuAtClass;
			--StuAtClass;
		}
		--now->Info->RefCount;
		if (--now->Info->RefCount == 0 && now->isAnonymous) // 如果匿名学生已经无人引用，垃圾回收
			delete now->Info;
		now->Info = NULL;
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
void Students::each(void callback(UINT ProductId, CString Name, CString StudentId, CString NumericId, bool isAtClass))
{
	Stu* curr = head->next;
	while (curr != NULL) {
		callback(curr->ProductId, curr->Info->Name, curr->Info->StudentId, curr->Info->NumericId, curr->isAtClass);
		curr = curr->next;
	}
}
/* @brief	遍历匿名学生
 * @param	回调函数，参数依次为答题器ID、数字学号
 */
void Students::eachAnonymous(void callback(UINT ProductId, CString NumericId, bool isAtClass))
{
	Stu* curr = head->next;
	while (curr != NULL) {
		if (curr->isAnonymous)
			callback(curr->ProductId, curr->Info->NumericId, curr->isAtClass);
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
    if (now = FindByProductId(ProductId)) { // 已经注册
		if (!now->isAtClass) { // 如果已经签到过了则不用重复签到
			now->isAtClass = true;
			++StuAtClass;
			if (now->Info->AtClassCount++ == 0 && !now->isAnonymous) // 如果名单中此人尚未到课，且在静态名单中，则需要更新名单到课人数
				++InfoList.StuAtClass;
			Sto->stuSignIn(ProductId); //保存签到信息到数据库
		}
        return true;
    } else { // 尚未注册
		return false;
	}
}
/* @brief	私有函数，添加学生到动态表
 * @note	学生必须不在动态表中，且已经被初始化
 */
void Students::AddToList(Stu* now) {
	now->next = head->next;
	head->next = now;
	if (now->isAnonymous)
		++AnonymousNum;
	++StudentCount;
}
/* @brief	私有函数，根据学号设置学生信息
 * @return	新学生是否在静态名单中
 * @note	假设原来学生是没有注册的。调用者必须在调用本方法前取消已存在的注册
 */
bool Students::SetInfoByNumericId(Stu* now, CString NumericId)
{
	StuStatic* info = InfoList.FindByNumericId(NumericId);
	if (info == NULL) { // 学生不在静态名单中
		if (!now->isAnonymous) { // 原来在静态名单中
			now->isAnonymous = true;
			++AnonymousNum;
		}
		now->Info = new StuStatic(NumericId); // 创建一个匿名学生
		now->Info->RefCount = 1;
		return false;
	} else { // 学生在静态名单中
		if (now->isAnonymous) { // 原来不在静态名单中
			now->isAnonymous = false;
			--AnonymousNum;
		}
		now->Info = info;
		++now->Info->RefCount;
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