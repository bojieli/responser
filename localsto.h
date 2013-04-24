/* 答题器本地存储
 * 用法：
 * 1. 对象构造时需要指定基站ID和安全标识符
 * 2. getCourses 获取各班级信息
 * 3. 用户选择班级后，setCurCourse 设置当前班级ID
 * 4. 调用 initStuNames 初始化班级学生列表
 * 5. 在学生签到时
 *    在每道题结束时调用 saveAnswers
 *    在老师设置正确答案时调用 saveCorAnswer
 */

#pragma once
#include "stdafx.h"
#include "sqlite3.h"
#include "Students.h"
#include "cloudconn.h"
#include "baseStation.h"
#include "courses.h"

#define SQL_MAXLEN 512

class LocalSto
{
	friend class Students;
protected:
	sqlite3 *dbconn;		//数据库连接
	int error;				//错误代码
	char *errmsg;			//操作失败的原因
	UINT course;			//课程ID，设置课程ID后才能开始一节课
	UINT lectureID;			//当前是第几节课，用于存储
private:
	UINT StationID;		    //基站ID
	CString StationToken;	//基站安全标识符
public:
	LocalSto(UINT StationID, CString StationToken);
	~LocalSto(void);
public:
	bool getCourses(Courses* c);			  //初始化课程信息数据结构
	bool beginCourse(UINT courseID);	      //开始一节课
	bool endCourse();						  //结束这节课
	bool addCourse(Course* c);				  //添加课程并获取课程ID
public:
	bool saveAnswers(Students* s);            //保存一道题的答题信息
	bool saveCorAnswer(Students* s);		  //保存正确答案
	bool setNumericId(CString NumericId, UINT ProductId); //设置学号
	bool stuSignIn(UINT ProductId);			  //学生签到
	bool initStuStaticList(Students* s);	  //初始化学生静态表
	bool initStudents(Students* s);			  //初始化学生动态表

private:
	CString rowsToStr(CString sql); // 将查询结果序列化出来
	CString rowsToStrIfNotChanged(CString sql); // 将未被上传过的查询结果序列化出来
	bool initDbFile();			//初始化数据库文件

	bool uploadToCloud();       //上传答题信息到云端，save 时会自动调用
	bool syncFromCloud();       //从云端同步学生姓名，实例化时会自动调用
	CString loadDataInStr(CString table, CString columns, const int column_count, CString str); // load data in file
private:
	bool insert(CString table, CString fields, CString data1, ...); // 插入字符串数据
	bool insert(CString table, CString fields, UINT data1, ...); // 插入整型数据
	bool getAll(CString table, int (*callback)(void*,int,char**,char**)); // 获取表中的所有数据，每条回调
	bool select(CString table, CString field, CString value, int (*callback)(void*,int,char**,char**));
	bool update(CString table, CString searchField, CString searchValue, CString updateField, CString updateValue);
	bool query(CString sql); // 原生数据库查询，不要返回值
	bool squery(CString pattern, ...); // query 与 Format 的结合
	bool query(CString sql, int (*callback)(void*,int,char**,char**), void* argtocallback); // 原生数据库查询
	CString selectFirst(CString pattern, ...); // 以字符串形式返回查询的第一个结果
};