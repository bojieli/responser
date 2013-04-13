#include "stdafx.h"
#include "courses.h"

Course::Course(UINT id, CString name, CString info)
{
	this->id = id;
	this->name = name;
	this->info = info;
	this->next = NULL;
}

Courses::Courses(void)
{
	this->head = NULL;
}
Courses::~Courses(void)
{
	Course* next;
	while (head != NULL) {
		next = head->next;
		delete head;
		head = next;
	}
}
void Courses::each(void callback(Course* c))
{
	Course* curr = head;
	while (curr != NULL) {
		callback(curr);
		curr = curr->next;
	}
}