// responser.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "responser.h"
#include "Students.h"
#include "courses.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

void show_course(Course* c) {
	printf("%u %s %s\n", c->id, c->name, c->info);
}
void show_stu(UINT ProductId, CString Name, CString StudentId)
{
	printf("ProductId=%u Name=%s StudentId=%s\n", ProductId, Name, StudentId);
}

void test() 
{
	BaseStation station;
	printf("new LocalSto...\n");
	LocalSto* sto = new LocalSto(station.ID(), station.token());
	printf("All courses:\n");
	Courses courses;
	sto->getCourses(&courses);
	courses.each(show_course);
	UINT course;
	printf("select course: ");
	scanf_s("%d", &course);
	Students* stu = new Students(sto, course);
	printf("Students Info:\n");
	stu->each(show_stu);
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
