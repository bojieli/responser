// responser.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "responser.h"
#include "Students.h"
#include "localsto.h"
#include "courses.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

void showCourse(Course* c) {
	wprintf_s(L"%u %s %s\n", c->id, c->name, c->info);
}
void showStu(UINT ProductId, CString Name, CString StudentId, CString NumericId)
{
	wprintf_s(L"[%u]\t%s\t%s\t%s\n", ProductId, Name, StudentId, NumericId);
}
void showStuAnonymous(UINT ProductId, CString NumericId)
{
	wprintf_s(L"[%u]\t%s\n", ProductId, NumericId);
}
void showStuStatic(CString Name, CString StudentId, CString NumericId, bool IsAtClass)
{	
	wprintf_s(L"%s\t%s\t%s\t", Name, StudentId, NumericId);
	if (IsAtClass)
		wprintf_s(L"IsAtClass");
	wprintf_s(L"\n");
}

void newCourse(Students* students, LocalSto* sto, Courses* courses)
{
	Course* newCourse = new Course(0, "hello", "world");
	if (sto->addCourse(newCourse))
		printf("add course OK\n");
	else
		printf("add course error\n");
	courses->add(newCourse);
	printf("new students info:\n");
	students->each(showStu);
}

void test() 
{
	BaseStation station;
	printf("new LocalSto...\n");
	LocalSto* sto = new LocalSto(station.ID(), station.token());
	Courses courses;
	sto->getCourses(&courses);
	printf("Total %d courses:\n", courses.Count);
	courses.each(showCourse);
	UINT course;
	printf("input course ID: ");
	scanf_s("%d", &course);
	Students* students = new Students(sto, course);
	printf("Info of %d static students:\n", students->InfoList.StuNum);
	students->InfoList.each(showStuStatic);
	printf("Info of %d online students:\n", students->OnlineStuNum);
	students->each(showStu);
	printf("Info of anonymous students:\n");
	students->eachAnonymous(showStuAnonymous);
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: 更改错误代码以符合您的需要
			_tprintf(_T("错误: MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			test();
		}
	}
	else
	{
		// TODO: 更改错误代码以符合您的需要
		_tprintf(_T("错误: GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
