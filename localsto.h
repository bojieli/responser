/* 答题器本地存储
 * 用法：
 * 1. 对象构造时需要指定基站ID和安全标识符
 * 2. getCourses 获取各班级信息
 * 3. 用户选择班级后，setCourseId 设置当前班级ID
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
	char *errmsg;			//操作失败的原因
	UINT course;			//课程号
	UINT lectureID;			//当前是第几节课，用于存储
private:
	UINT StationID;		    //基站ID
	CString StationToken;	//基站安全标识符
public:
	LocalSto(UINT StationID, CString StationToken);
	~LocalSto(void);
	bool getCourses(Courses* c);			  //初始化课程信息数据结构
	bool setCourseId(UINT course_id);		  //设置当前课程ID，设置后才能保存答题信息
public:
	bool saveAnswers(Students* s);            //保存一道题的答题信息
	bool saveCorAnswer(Students* s);		  //保存正确答案
	bool setNumericId(CString NumericId, UINT ProductId); //设置学号
	bool stuSignIn(UINT ProductId);			  //学生签到
	bool initStuNames(Students* s);			  //初始化学生姓名数据结构
private:
	CString rowsToStr(const char* sql); // 将查询结果序列化出来
	bool initDbFile();			//初始化数据库文件
	bool addCourse(CString name, CString info);	//添加课程
	bool uploadToCloud();       //上传答题信息到云端，save 时会自动调用
	bool syncFromCloud();       //从云端同步学生姓名，实例化时会自动调用
	char* loadDataInStr(const char* table, const char* columns, const int column_count, char* str); // load data in file
private:
	bool insert(const char* table, const char* fields, char* data1, ...); // 插入字符串数据
	bool insert(const char* table, const char* fields, UINT data1, ...); // 插入整型数据
	bool getAll(const char* table, int (*callback)(void*,int,char**,char**)); // 获取表中的所有数据，每条回调
	bool select(const char* table, const char* field, char* value, int (*callback)(void*,int,char**,char**));
	bool update(const char* table, const char* searchField, char* searchValue, const char* updateField, char* updateValue);
	bool query(const char* sql); // 原生数据库查询，不要返回值
	bool squery(const char* pattern, ...); // query与sprintf的结合
	bool query(const char* sql, int (*callback)(void*,int,char**,char**), void* argtocallback); // 原生数据库查询
	char* selectFirst(const char* pattern, ...); // 以字符串形式返回查询的第一个结果
};