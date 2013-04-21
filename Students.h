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


allStu提供给外部的接口函数如下：
针对StuStatic m_List,它只需要在老师选择完班级之后添加即可，因此只有一个接口
allStu.m_List.NewStuStatic()

public: //答题操作
	void Start(); //开始答题
	bool End(); //结束答题，若发送到服务器则返回1，若保存到U盘则返回0

	int USBAddAnswer(BYTE* ProductID, BYTE ANS, UINT ansTime); 
	在答题模式和课前模式中使用，在开启课前模式时调用开始答题，来判断哪些学生已经到了

	void USBAddCorAnswer(BYTE ANS);//添加正确答案，教师笔或者按键提供
public: //注册操作
	bool USBRegister(BYTE* ID,BYTE* ProductID); //注册到课堂


用到的数据接口
public: //答题基本情况
	int QuesTotal;//题目总数,显示所需要的数据
	int StudTotal;//学生总数，显示所需要的数据
	int StuAtClass;//到位的学生总数，显示所需要的数据
	int StuAlreadyAns;//已经答题的学生数目，显示所需要的数据
	bool isStarted;//是否处于答题状态，显示所需要的数据
	BYTE AnswerCount[64];//记录每一种答案的数目，总共可以选择A B C D E F 六个答案 0x00~0x3f;//显示所需要的数据

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
	UINT ProductId;			//答题器ID
	Stu* next;				//学生链表下一元素
public: // 学生作答信息
	BYTE Ans;				//最近一次作答的答案
	UINT AnsTime;			//最近一次答题所需时间
	BYTE mark;				//评分
	bool IsAtClass;			//是否在课堂上
public: 
	Stu(UINT ProductId);
	~Stu(void);
};

class Students
{
	friend class LocalSto;
public:
	UINT course;			//班级编号
	LocalSto* Sto;			//数据库连接
	Stu* head;				//学生链表，head 是哨兵
	StuStaticList InfoList;	//学生静态信息链表
	UINT beginTime;			//答题开始时间
	BYTE CorAnswer;			//最近一次正确答案
	int QuesTotal;			//题目总数
	int StudTotal;			//学生总数
	int StuAtClass;			//到位的学生总数
	int StuAlreadyAns;		//已经答题的学生数目
	bool isStarted;			//是否处于答题状态
	UINT AnswerCount[64];//记录每一种答案的数目，总共可以选择A B C D E F 六个答案 0x00~0x3f;//显示所需要的数据
public: //选定班级后实例化
	Students(LocalSto* sto, UINT course);
	~Students(void);
public: //鼠标操作
	void Start(); //开始答题
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
	void each(void callback(UINT ProductId, CString Name, CString StudentId));
private:
	bool AddAnswer(UINT ProductId, BYTE ANS, UINT AnsTime); //学生答题
	bool AddCorAnswer(BYTE ANS); //添加正确答案
	bool SignIn(UINT ProductId); //答题器签到
	bool SetInfoByNumericId(Stu* now, CString NumericId);
	Stu* AddAnonymous(UINT ProductId);
	Stu* FindByProductId(UINT ProductId);
};