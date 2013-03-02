#include "stdafx.h"
#include "localsto.h"

LocalSto::LocalSto()
{
	if (SQLITE_OK != sqlite3_open("test.db", &dbconn)) {
		Error(E_FATAL, _T("无法打开数据库"));
		return;
	}
	if (SQLITE_OK != this->query(".read initdb.sql")) {
		Error(E_NOTICE, _T("找不到数据库初始化文件，无法初始化数据库"));
	}
	this->syncFromCloud();
	this->squery("INSERT INTO lecture (begin_time,synced) VALUES (%ld,0)", time(NULL));
	this->lectureID = (long)sqlite3_last_insert_rowid(this->dbconn);
}
LocalSto::~LocalSto()
{
	this->squery("UPDATE lecture SET end_time=%ld WHERE id=%d", time(NULL), this->lectureID);
	if (SQLITE_OK != sqlite3_close(dbconn)) {
		Error(E_WARNING, _T("无法正常关闭数据库"));
	}
}
static long toLong(BYTE* s, int count)
{
	long l = 0;
	for (int i=0; i<count; i++)
		l = (l<<8) + s[i];
	return l;
}
static BYTE* toBytes(long num, int count)
{
	BYTE *buf = (BYTE*)malloc(count);
	for (int i=count-1; i>=0; i--) {
		buf[i] = num & 0xFF;
		num>>=8;
	}
}
bool LocalSto::save(Students *s)
{
	long currTime = (long)time(NULL);
	this->insert("problem", "lecture,problem,begin_time,end_time,corrent_ans",
		this->lectureID, s->QuesTotal, s->beginTime, currTime, s->CorAnswer.Ans);
	Stu* stu = s->head;
	while ((stu = stu->next) != NULL) {
		this->insert("answer", "lecture,problem,product,answer,ans_time",
			this->lectureID, s->QuesTotal, toLong(stu->ProductID,4), stu->Ans, stu->ansTime);
	}
}
bool LocalSto::stuRegister(Stu* stu)
{
	this->squery("REPLACE INTO product (product_id,student_id) VALUES ('%ld','%ld')", toLong((BYTE *)(LPCTSTR)stu->ProductID, 4), toLong((BYTE *)stu->ID, 5));
}

char* addslashesForSpace(char* str);
char* stripslashesForSpace(char* str, char** newstr);

CString LocalSto::rowsToStr(const char* sql)
{
	CString str;
	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare(dbconn, sql, -1, &stmt, (const char**)&errmsg);
	int col_count = sqlite3_column_count(stmt);
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		for (int i=0; i<col_count; i++) {
			if (i>0)
				str += CString(_T("\t"));
			str += CString(addslashesForSpace((char *)sqlite3_column_text(stmt, i)));
		}
		str += CString(_T("\n"));
	}
	sqlite3_finalize(stmt);
	return str;
}
bool LocalSto::uploadToCloud()
{
	CString data;
	data += this->rowsToStr("SELECT product_id,student_id FROM product");
	data += "\n";
	data += this->rowsToStr("SELECT id,begin_time,end_time FROM lecture WHERE synced=0");
	data += "\n";
	data += this->rowsToStr("SELECT product,lecture,reg_time FROM register");
	data += "\n";
	data += this->rowsToStr("SELECT lecture,problem,begin_time,end_time,corrent_ans FROM problem");
	data += "\n";
	data += this->rowsToStr("SELECT lecture,problem,product,ans,ans_time FROM answer");

	CloudConn *cloud = new CloudConn("upload");
	cloud->SetBody(CString(L"data"), data);
	CString response = cloud->send();
	if (response == "OK") {
		this->query("DELETE FROM register; DELETE FROM problem; DELETE FROM answer; UPDATE lecture SET synced=1");
		return true;
	}
	if (response == "FAIL")
		Error(E_WARNING, _T("上传到云端过程中内部错误"));
	else
		Error(E_NOTICE, _T("无法连接到云端"));
	return false;
}
bool LocalSto::syncFromCloud()
{
	CloudConn *cloud = new CloudConn("sync");
	char *response = (LPSTR)(LPCTSTR)cloud->send();
	if (response == NULL || *response == '\0') {
		Error(E_NOTICE, _T("无法连接到云端"));
		return false;
	}
	// 下载得到学号到姓名的映射，保存到本地数据库
	this->query("DELETE FROM student");
	while (*response) {
		char *student_id, *name;
		response = stripslashesForSpace(response, &student_id);
#define ASSERT_TRANS(expr) if (!(expr)) { \
			Error(E_WARNING, _T("从云端下载数据时内部错误")); \
			return false; \
		}

		ASSERT_TRANS(*response++ == '\t');
		response = stripslashesForSpace(response, &name);
		ASSERT_TRANS(*response++ == '\t');
#undef ASSERT_TRANS

		this->insert("student", "student_id,name", student_id, name);
	}
	return true;
}
bool LocalSto::initStuNames(StuStaticList* m_List)
{
	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare(dbconn, "SELECT student_id,name FROM student", -1, &stmt, (const char **)&errmsg);
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		long student_id = (long)sqlite3_column_int64(stmt, 0);
		char *name = (char *)sqlite3_column_text(stmt, 1);
		m_List->NewStuStatic(CString(name), (BYTE*)toBytes(student_id,5)); // 姓名是字符串，学号是5字节整数
	}
	sqlite3_finalize(stmt);
}

// 下面是私有函数
static char* addslashes(char* str)
{
	char *newstr = (char *)malloc(strlen(str)*2+1);
	char *end = newstr;
	while (*str != '\0') {
		if (*str == '\'' || *str == '\"' || *str == '\\')
			*end++ = '\\';
		*end++ = *str++;
	}
	*end++ = '\0';
	return (char *)realloc(newstr, end - newstr);
}
static char* addslashesForSpace(char* str)
{
	char *newstr = (char *)malloc(strlen(str)*2+1);
	char *end = newstr;
	while (*str != '\0') {
		if (*str == '\t' || *str == '\n' || *str == '\\')
			*end++ = '\\';
		*end++ = *str++;
	}
	*end++ = '\0';
	return (char *)realloc(newstr, end - newstr);
}
static char* stripslashesForSpace(char* str, char** newstr)
{
	char *s = (char *)malloc(SQL_MAXLEN);
	int length = SQL_MAXLEN;
	char *end = s;
	while (*str != '\0') {
		if (*str == '\t' || *str == '\n')
			break;
		if (*str == '\\')
			*str++;
		*end++ = *str++;
		if (end - s >= length-1) {
			length += SQL_MAXLEN;
			int currlen = end - s;
			s = (char *)realloc(s, length);
			end = currlen + s;
		}
	}
	*end++ = '\0';
	s = (char *)realloc(s, end - s);
	*newstr = s;
	return str;
}
bool LocalSto::getAll(const char* table, int (*callback)(void*,int,char**,char**))
{
	char sql[SQL_MAXLEN];
	sprintf_s(sql, "SELECT * FROM %s", table);
	return this->query(sql, callback, NULL);
}
bool LocalSto::select(const char* table, const char* field, char* value, int (*callback)(void*,int,char**,char**))
{
	char sql[SQL_MAXLEN];
	sprintf_s(sql, "SELECT * FROM %s WHERE %s='%s'", table, field, addslashes(value));
	return this->query(sql, callback, NULL);
}
bool LocalSto::update(const char* table, const char* searchField, char* searchValue, const char* updateField, char* updateValue)
{
	return this->squery("UPDATE %s SET %s='%s' WHERE %s='%s'",
		table,
		updateField, addslashes(updateValue), 
		searchField, addslashes(searchValue));
}
bool LocalSto::query(const char* sql)
{
	return (SQLITE_OK == sqlite3_exec(dbconn, sql, NULL, 0, &errmsg));
}
bool LocalSto::squery(const char* format, ...)
{
	char sql[SQL_MAXLEN];
	va_list args;
	va_start(args, format);
	vsprintf(sql, format, args);
	va_end(args);
	return LocalSto::query(sql);
}
bool LocalSto::query(const char* sql, int (*callback)(void*,int,char**,char**), void* argtocallback)
{
	return (SQLITE_OK == sqlite3_exec(dbconn, sql, callback, argtocallback, &errmsg));
}

bool LocalSto::insert(const char* table, const char* fields, char* data1, ...)
{
	char sql[SQL_MAXLEN];
	sprintf(sql, "INSERT INTO %s (%s) VALUES (", table, fields);
	va_list args;
	va_start(args, data1);
	bool isFirst = true;
	while (char* data = va_arg(args, char*)) {
		if (isFirst)
			isFirst = false;
		else
			strncat(sql, ",", SQL_MAXLEN);
		strncat(sql, "'", SQL_MAXLEN);
		strncat(sql, addslashes(data), SQL_MAXLEN);
		strncat(sql, "'", SQL_MAXLEN);
	}
	va_end(args);
	strncat(sql, ")", SQL_MAXLEN);
	return this->query(sql);
}
bool LocalSto::insert(const char* table, const char* fields, long data1, ...)
{
	char sql[SQL_MAXLEN] = {0};
	char* end;
	sprintf(sql, "INSERT INTO %s (%s) VALUES (", table, fields);
	end = sql + strlen(sql);
	va_list args;
	va_start(args, data1);
	bool isFirst = true;
	while (long data = va_arg(args, long)) {
		if (isFirst)
			isFirst = false;
		else
			*end++ = ',';
		sprintf(end, "%ld", data);
		while (*end++ != '\0');
	}
	va_end(args);
	*end++ = ')';
	*end++ = '\0';
	return this->query(sql);
}
