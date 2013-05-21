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
void showStu(UINT ProductId, CString Name, CString StudentId, CString NumericId, bool isAtClass)
{
	wprintf_s(L"[%u]\t%s\t%s\t%s", ProductId, Name, StudentId, NumericId);
	if (isAtClass)
		wprintf_s(L"\tIsAtClass");
	wprintf_s(L"\n");
}
void showStuAnonymous(UINT ProductId, CString NumericId, bool isAtClass)
{
	wprintf_s(L"[%u]\t%s", ProductId, NumericId);
	if (isAtClass)
		wprintf_s(L"\tIsAtClass");
	wprintf_s(L"\n");
}
void showStuStatic(StuStatic* s)
{	
	wprintf_s(L"%s\t%s\t%s\t", s->Name, s->StudentId, s->NumericId);
	if (s->AtClassCount == 1)
		wprintf_s(L"IsAtClass");
	else if (s->AtClassCount > 1)
		wprintf_s(L"IsAtClass (%d instances)", s->AtClassCount);
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

#define DO_TEST(welcome,expr) \
	printf(welcome "... "); \
	if (expr) \
		printf("OK\n"); \
	else \
		printf("failed\n");

bool testRegister(Students* stu, UINT ProductId, CString NumericId)
{
	printf("Register [%u] %s ", ProductId, NumericId);
	if (stu->Register(NumericId, ProductId)) {
		printf("OK\n");
		return true;
	}
	else {
		printf("Error\n");
		return false;
	}
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
	printf("Starting course...\n");
	Students* students = new Students(sto, course);

	testRegister(students, 0x02, _T("0110007146"));

	DO_TEST("start problem", students->Start())
	DO_TEST("add answer", students->USBAddAnswer(0x02, 10))
	DO_TEST("add correct answer", students->USBAddCorAnswer(20))
	DO_TEST("end of problem", students->End())

	
	printf("\n%d static students, %d online:\n",
		students->GetStaticStuTotal(), students->GetStaticStuAtClass());
	students->InfoList.each(showStuStatic);
	printf("\n%d dynamic students, %d online, %d already answered:\n",
		students->GetStuTotal(), students->GetStuAtClass(), students->GetStuAlreadyAns());
	students->each(showStu);
	printf("\n%d anonymous students:\n", students->GetAnonymousNum());
	students->eachAnonymous(showStuAnonymous);

	delete students;
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
