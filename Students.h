/******************************************文件说明*****************************************
Students.h 以及 Students.cpp 主要用于存储学生答题

整个数据更改流程：
1、当老师允许注册时，学生才能输入学号。注册时学生将学号和产品ID绑定在一起，以后学生只需要用产品ID即可。此时上位机只接收注册帧。
老师联网将学生的姓名和学号下载到u盘，我们收到产品ID后检测是否已经有了学号。此时会出现以下几种情况（软件同时收到产品ID和学号）：
学生手里拿着一个答题器，也就是一个产品ID，去申索学号（资格）。此时机器里面已经有了一个学号姓名映射表。
当学生注册一个产品ID时，去寻找名单里面是否有他的名字，如果有，则将他的姓名和学号修改了。
当学生注册一个产品ID时，如果没有，这学号按照新的学号，姓名改为匿名。
2、当老师不允许注册时。也就是产品ID没有注册到表格里面，那么这个产品是不能答题了。虽然基站会把数据传到上位机，但上位机不做任何处理。
3、增加一种即席模式，也就是不需要任何注册就能凭借ID答题。
平时当老师不允许注册时，是不会有新的学生加入的。

API参见 responser.cpp 中的调用示例。

********************************************文件说明****************************************/

#pragma once
#include "stdafx.h"
#include "afxmt.h"
#include "localsto.h"

class StuStatic
{
public:
	CString Name;		//姓名
	CString StudentId;	//学号
	CString NumericId;	//数字学号
	int AtClassCount;	//在课堂上的实例计数
	int RefCount;		//被动态表引用的计数
	StuStatic* next;
public:
	StuStatic(CString Name, CString StudentID, CString NumericId); // 新建普通学生
	StuStatic(CString NumericId); // 新建匿名学生
	StuStatic(void); // 新建哨兵
	~StuStatic(void);
};

class StuStaticList
{
public:
	StuStaticList(void);
	~StuStaticList(void);
public: //查找学生
	StuStatic* FindByStudentId(CString StudentId);
	StuStatic* FindByNumericId(CString NumericId);
public: //添加学生
	StuStatic* Add(CString Name, CString StudentId, CString NumericId);
public: //遍历名单
	void each(void callback(StuStatic* s));

public:
	StuStatic* head;
	int StuNum;				//名单中的学生总数
	int StuAtClass;			//名单中在课堂的学生总数
};

class Stu
{
public:
	StuStatic* Info;		//学生静态信息
	UINT ProductId;			//答题器ID
	bool isAnonymous;		//是否匿名
	bool isAtClass;			//是否在课堂上
	Stu* next;				//学生链表下一元素
public: // 学生作答信息
	BYTE Ans;				//最近一次作答的答案
	UINT AnsTime;			//最近一次答题所需时间
	BYTE mark;				//评分
public: 
	Stu(UINT ProductId);
	~Stu(void);
};

class Students
{
	friend class LocalSto;
public: //选定班级后实例化
	Students(LocalSto* sto, UINT course);
	~Students(void);
public: //鼠标操作
	bool Start(); //开始答题
	bool End(); //结束答题
public: //答题器接口操作
	bool USBAddAnswer(UINT ProductId, BYTE ANS); // USB答题和签到
	bool USBAddCorAnswer(BYTE ANS); //添加正确答案
	bool Register(CString NumericId, UINT ProductId); //答题器设置学号，注册模式
	bool TeacherMark(UINT ProductId, BYTE mark); // 老师给学生评分
public: //从数据库初始化
	bool Add(CString NumericId, UINT ProductId);
public: //遍历学生
	void each(void callback(Stu* stu));
	void each(void callback(UINT ProductId, CString Name, CString StudentId, CString NumericId, bool isAtClass)); //遍历已签到的学生
	void eachAnonymous(void callback(UINT ProductId, CString NumericId, bool isAtClass)); //遍历匿名学生
public: // 人数统计
	int GetStuTotal(void) {return StudentCount;} // 动态表中学生总数
	int GetStaticStuTotal(void) {return InfoList.StuNum;} // 静态表中学生总数
	int GetStuAtClass(void) {return StuAtClass;} // 到课学生总数
	int GetStaticStuAtClass(void) {return InfoList.StuAtClass;} // 静态表中到课学生总数
	int GetAnonymousNum(void) {return AnonymousNum;} // 匿名学生数
	int GetStuAlreadyAns(void) {return StuAlreadyAns;} // 已经答题的学生数

public:
	UINT course;			//班级编号
	LocalSto* Sto;			//数据库连接
	Stu* head;				//学生链表，head 是哨兵
	StuStaticList InfoList;	//学生静态信息链表
	UINT beginTime;			//答题开始时间
	BYTE CorAnswer;			//最近一次正确答案
	int QuestionNum;		//题目总数
	bool isStarted;			//是否处于答题状态
	UINT AnswerCount[64];   //记录每一种答案的数目，总共可以选择A B C D E F 六个答案 0x00~0x3f
private:
	int StudentCount;		//动态表中的学生总数
	int StuAtClass;			//到位的学生总数
	int AnonymousNum;		//匿名学生总数
	int StuAlreadyAns;		//已经答题的学生数目

private: // ===== 以下是私有函数 =====
	void AddToList(Stu* stu);
	bool AddAnswer(UINT ProductId, BYTE ANS, UINT AnsTime); //学生答题
	bool AddCorAnswer(BYTE ANS); //添加正确答案
	bool SignIn(UINT ProductId); //答题器签到
	bool SetInfoByNumericId(Stu* now, CString NumericId);
	Stu* NewStu(UINT ProductId);
	Stu* FindByProductId(UINT ProductId);
};