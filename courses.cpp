#include "stdafx.h"
#include "courses.h"

Course::Course(UINT id, CString name, CString info)
{
	this->id = id;
	this->name = name;
	this->info = info;
	this->next = NULL;
}
Course::Course(void)
{
	this->next = NULL;
}

Courses::Courses(void)
{
	this->head = new Course();
	this->Count = 0;
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
	Course* curr = head->next;
	while (curr != NULL) {
		callback(curr);
		curr = curr->next;
	}
}
void Courses::add(Course* c)
{
	c->next = head->next;
	head->next = c;
	this->Count++;
}