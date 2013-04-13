// responser.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "responser.h"
#include "Students.h"
#include "courses.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ψһ��Ӧ�ó������

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
		// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: ���Ĵ�������Է���������Ҫ
			_tprintf(_T("����: MFC ��ʼ��ʧ��\n"));
			nRetCode = 1;
		}
		else
		{
			test();
		}
	}
	else
	{
		// TODO: ���Ĵ�������Է���������Ҫ
		_tprintf(_T("����: GetModuleHandle ʧ��\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
