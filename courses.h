#pragma once

class Course {
public:
	Course(UINT id, CString name, CString info);
	UINT id;
	CString course_id;
	CString name;
	UINT lecture_count;
	CString info;
	Course* next;
};

class Courses {
public:
	int Count; // �γ���Ŀ
	Course* head;
	Courses(void);
	~Courses(void);
	void each(void callback(Course* c));
	void add(Course* c);
};