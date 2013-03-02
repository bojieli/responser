#pragma once
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

#define ID_LENGTH 5
#define PID_LENGTH 4

class StuStatic
{
public:
	StuStatic(CString Name1,BYTE* ID1)
	{
		Name = Name1;
		for(int i=0;i<ID_LENGTH;i++)
			ID[i] = ID1[i];
		next = NULL;
	}
	StuStatic()
	{
		next = NULL;
	}
	~StuStatic(void)
	{
		
	}
public:
	CString Name;
	BYTE ID[5];
	StuStatic* next;
	bool isMe(BYTE* ID1)//输入ID是否一样,用于查找
	{
		
		for(int i = 0; i<ID_LENGTH; i++)
		if (ID1[i] != this->ID[i])
			return false;
			return true;
	}
};
class StuStaticList
{
public:
	StuStaticList()
	{
		StaticList = new StuStatic();
	}
	~StuStaticList()
	{
		while (this->StaticList != NULL)
		{
			StuStatic *temp = this->StaticList;
			this->StaticList = temp->next;
			delete temp;
		}
	}
public:
	StuStatic* StaticList;
	void NewStuStatic(CString Name,BYTE* ID)
	{
		StuStatic* now = new StuStatic(Name,ID);
		now->next = StaticList->next;
		StaticList->next = now;
	}
	StuStatic* FindStu(BYTE* ID1)
	{
		StuStatic* temp = StaticList;
		while((temp = temp->next)!=NULL)
		{
			if(temp->isMe(ID1))
				return temp;
		}
		return 0;
	}

};

class Stu
{
public: 
	Stu(void);
	Stu(BYTE* ID1,BYTE* ProductID1,CString Name1);
	~Stu(void);
	Stu* next; //学生链表下一元素

public: //一个学生的基本信息
	BYTE ID[5];//学生学号用5个BYTE表示
	BYTE ProductID[4];//学生注册时的产品ID Product ID
	CString Name;//学生的名字
	BYTE Ans;//当Ans为0时表示没有作答
	unsigned int ansTime;//答题所需要的时间
	bool IsAtClass;
	bool isMyProduct(BYTE* ProductID1);//输入ProductID是否一样，用于查找
	
};

class Students
{
public:
	Students(void);
	~Students(void);
protected:
	Stu* head; //学生链表
public:
	Stu* Add(BYTE* ID,BYTE* ProductID, CString Name);//添加学生
	Stu* Find(BYTE* ProductID);
	Stu CorAnswer;//存储正确答案
	StuStaticList m_List;//静态名单
public: //答题基本情况
	int QuesTotal;//题目总数,显示所需要的数据
	int StudTotal;//学生总数，显示所需要的数据
	int StuAtClass;//到位的学生总数，显示所需要的数据
	int StuAlreadyAns;//已经答题的学生数目，显示所需要的数据
	bool isStarted;//是否处于答题状态，显示所需要的数据
	BYTE AnswerCount[64];//记录每一种答案的数目，总共可以选择A B C D E F 六个答案 0x00~0x3f;//显示所需要的数据
public: //答题操作
	void Start(); //开始答题
	bool End(); //结束答题，若发送到服务器则返回1，若保存到U盘则返回0
	int USBAddAnswer(BYTE* ProductID, BYTE ANS, unsigned int ansTime); //添加答案，返回 1 表示原有，返回 0 表示新增，返回 -1 表示不在答题时间
	void USBAddCorAnswer(BYTE ANS);//添加正确答案
public: //注册操作
	bool USBRegister(BYTE* ID,BYTE* ProductID); //注册到课堂
};

extern Students allStu;
extern CCriticalSection m_Lock;//线程同步