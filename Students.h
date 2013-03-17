#pragma once
#include "stdafx.h"
#include "afxmt.h"
#include "localsto.h"
/******************************************文件说明*****************************************
Students.h 以及 Students.cpp 主要用于存储学生答题
结构体 Answer 存储一个答案
类 Stu 存储一个学生的基本信息以及他的答案
类 Students 存储一系列学生以及对外的接口，只有一个实例
另外，整个通信函数也都全部在这里面

allStu 为整个工程的全局变量

对外接口如下：
每次开始答题时调用 allStu.Start(); 初始化数据库
收到一个学生的答题结果后调用 allStu.AddAnswer(BYTE* ID,Answer* ANS)

整个数据更改流程：
1、当老师允许注册时，学生才能输入学号。注册时学生将学号和产品ID绑定在一起，以后学生只需要用产品ID即可。此时上位机只接收注册帧。
老师联网将学生的姓名和学号下载到u盘，我们收到产品ID后检测是否已经有了学号。此时会出现以下几种情况（软件同时收到产品ID和学号）：
学生手里拿着一个答题器，也就是一个产品ID，去申索学号（资格）。此时机器里面已经有了一个学号姓名映射表。
当学生注册一个产品ID时，去寻找名单里面是否有他的名字，如果有，则将他的姓名和学号修改了。
当学生注册一个产品ID时，如果没有，这学号按照新的学号，姓名改为匿名。
2、当老师不允许注册时。也就是产品ID没有注册到表格里面，那么这个产品是不能答题了。虽然基站会把数据传到上位机，但上位机不做任何处理。
3、增加一种即席模式，也就是不需要任何注册就能凭借ID答题。
平时当老师不允许注册时，是不会有新的学生加入的。

********************************************文件说明****************************************/

class StuStatic
{
public:
	CString Name;		//姓名
	CString StudentId;	//学号
	CString NumericId;	//数字学号
	StuStatic* next;
public:
	StuStatic(void);
	StuStatic(CString Name, CString StudentID, CString NumericId);
	~StuStatic(void);
};

class StuStaticList
{
public:
	StuStatic* head;
public: //查找学生
	StuStatic* FindByStudentId(CString StudentId);
	StuStatic* FindByNumericId(CString NumericId);
public: //添加学生
	StuStatic* Add(CString Name, CString StudentId, CString NumericId);
public:
	StuStaticList(void);
	~StuStaticList(void);
};

class Stu
{
public:
	StuStatic* Info;		//学生静态信息
	long ProductId;		//答题器ID
	Stu* next;			//学生链表下一元素
public: // 学生作答信息
	BYTE Ans;			//最近一次作答的答案
	unsigned int AnsTime; //最近一次答题所需时间
	bool IsAtClass;		//是否在课堂上
public: 
	Stu(long ProductId);
	~Stu(void);
};

class Students
{
	friend class LocalSto;
public:
	CString course;			//班级编号
	LocalSto* Sto;			//数据库连接
	Stu* head;				//学生链表
	StuStaticList InfoList;	//学生静态信息链表
	unsigned int beginTime;	//开始时间
	BYTE CorAnswer;			//最近一次正确答案
	int QuesTotal;			//题目总数
	int StudTotal;			//学生总数
	int StuAtClass;			//到位的学生总数
	int StuAlreadyAns;		//已经答题的学生数目
	bool isStarted;			//是否处于答题状态
	unsigned int AnswerCount[64];//记录每一种答案的数目，总共可以选择A B C D E F 六个答案 0x00~0x3f;//显示所需要的数据
public: //选定班级后实例化
	Students(CString course);
	~Students(void);
public: //鼠标操作
	void Start(); //开始答题
	bool End(); //结束答题
public: //答题器接口操作
	bool AddAnswer(long ProductId, BYTE ANS, unsigned int AnsTime); //学生答题
	bool AddCorAnswer(BYTE ANS);   //添加正确答案
	bool Register(long ProductId); //答题器签到
	bool SetNumericId(CString NumericId, long ProductId); //答题器设置学号
public: //从数据库初始化
	bool Add(CString NumericId, long ProductId);
private:
	bool SetInfoByNumericId(Stu* now, CString NumericId);
	Stu* AddAnonymous(long ProductId);
	Stu* FindByProductId(long ProductId);
};

extern Students allStu;
extern CCriticalSection m_Lock;//线程同步