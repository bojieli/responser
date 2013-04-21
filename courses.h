#pragma once

class Course {
public:
	Course(UINT id, CString name, CString info);
	Course(void); // 用于哨兵
	UINT id;
	CString course_id;
	CString name;
	UINT lecture_count;
	CString info;
	Course* next;
};

class Courses {
public:
	int Count; // 课程数目
	Course* head; // head 是哨兵
	Courses(void);
	~Courses(void);
	void each(void callback(Course* c));
	void add(Course* c);
};