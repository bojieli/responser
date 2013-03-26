#include "stdafx.h"
#include "localsto.h"

static char* addslashes(char* str);
static char* addslashesForSpace(char* str);
static char* stripslashesForSpace(char* str, char** newstr);
static long toLong(BYTE* s, int count);
static BYTE* toBytes(long num, int count);

LocalSto::LocalSto(char* course_id)
{
	this->course = addslashes(course_id);
	if (SQLITE_OK != sqlite3_open("test.db", &dbconn)) {
		Error(E_FATAL, _T("�޷������ݿ�"));
		return;
	}
	if (!this->initDbFile()) {
		Error(E_FATAL, _T("�޷���ʼ�����ݿ�ṹ"));
		return;
	}
	this->syncFromCloud();
	int lecture_count = atoi(this->selectFirst("SELECT lecture_count FROM course WHERE course_id='%s'", course));
	if (!this->squery("UPDATE course SET lecture_count=lecture_count+1 WHERE course_id='%s'", course))
		goto error;
	if (!this->squery("INSERT INTO lecture (course,id,begin_time) VALUES ('%s',%d,%ld)", course, lecture_count+1, time(NULL)))
		goto error;
	this->lectureID = (long)sqlite3_last_insert_rowid(this->dbconn);
	return;
error:
	Error(E_FATAL, _T("���ݿ��ʼ��������Ϣʧ��"));
	return;
}
LocalSto::~LocalSto()
{
	this->squery("UPDATE lecture SET end_time=%ld WHERE course='%s' AND id=%d", time(NULL), course, lectureID);
	if (SQLITE_OK != sqlite3_close(dbconn)) {
		Error(E_WARNING, _T("�޷������ر����ݿ�"));
	}
}
/* @brief	����һ����Ĵ�����Ϣ
 * @param	s ���ö���
 * @return	�Ƿ񱣴�ɹ�
 */
bool LocalSto::saveAnswers(Students *s)
{
	long currTime = (long)time(NULL);
	if (!this->squery("INSERT INTO problem (course,lecture,problem,begin_time,end_time,correct_ans) "
		"VALUES ('%s',%d,%d,%d,%d,%d)",
		course, lectureID, s->QuesTotal, s->beginTime, currTime, s->CorAnswer))
		return false;
	Stu* stu = s->head;
	bool success = true;
	while ((stu = stu->next) != NULL) {
		if (!this->squery("INSERT INTO answer (course,lecture,problem,product,answer,ans_time) "
			"VALUES ('%s',%d,%d,%d,%d,%d)",
			course, lectureID, s->QuesTotal, stu->ProductId, stu->Ans, stu->AnsTime))
			success = false;
	}
	if (success)
		this->uploadToCloud();
	return success;
}
/* @brief	������ȷ��
 * @param	s ���ö���
 * @return	�Ƿ񱣴�ɹ�
 * @note	�˺���ֻ����A����������B����δ��ʼʱ���ã�����A�����ȷ��
 */
bool LocalSto::saveCorAnswer(Students *s)
{
	return this->squery("UPDATE problem SET correct_ans=%d WHERE course='%s' AND lecture=%d AND problem=%d",
		course, s->CorAnswer, lectureID, s->QuesTotal);
}
/* @brief	ѧ��ǩ��
 * @param	ProductId ��ƷID
 * @return	�Ƿ񱣴�ɹ�
 */
bool LocalSto::stuRegister(long ProductId)
{
	return this->squery("INSERT INTO register (course,lecture,product,reg_time)"
		"VALUES ('%s',%d,%d,%d)",
		course, lectureID, ProductId, time(NULL));
}
/* @brief	����ѧ�����õ�ѧ��
 * @param	NumericId ����ѧ��
 * @param	ProductId ��ƷID
 * @return	�Ƿ񱣴�ɹ�
 */
bool LocalSto::setNumericId(CString NumericId, long ProductId)
{
	return this->squery("REPLACE INTO product (product_id,numeric_id) VALUES ('%ld','%s')",
		ProductId, (LPCTSTR)NumericId);
}
/* @brief	�����ݿ��ѯ�����ʾ���ַ���
 * @param	sql ���ݿ��ѯ
 * @return	��ѯ�ṹ
 */
CString LocalSto::rowsToStr(const char* sql)
{
	CString str;
	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare(dbconn, sql, -1, &stmt, (const char**)&errmsg);
	int col_count = sqlite3_column_count(stmt);
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		for (int i=0; i<col_count; i++) {
			if (i>0)
				str += CString(L"\t");
			str += CString(addslashesForSpace((char *)sqlite3_column_text(stmt, i)));
		}
		str += CString(L"\n");
	}
	sqlite3_finalize(stmt);
	return str;
}
/* @brief	���������ݿ��ѧ��ǩ����Ϣ�ʹ�����Ϣ�ϴ����ƶ�
 * @return	�Ƿ��ϴ��ɹ�
 */
bool LocalSto::uploadToCloud()
{
	CString data = baseStation.Id() + "\n";

	data += this->rowsToStr("SELECT id,lecture_count FROM course");
	data += "\n";
	data += this->rowsToStr("SELECT product_id,numeric_id FROM product");
	data += "\n";
	data += this->rowsToStr("SELECT course,id,begin_time,end_time FROM lecture");
	data += "\n";
	data += this->rowsToStr("SELECT course,product,lecture,reg_time FROM register");
	data += "\n";
	data += this->rowsToStr("SELECT course,lecture,problem,begin_time,end_time,corrent_ans FROM problem");
	data += "\n";
	data += this->rowsToStr("SELECT course,lecture,problem,product,ans,ans_time FROM answer");

	CloudConn *cloud = new CloudConn("upload");
	cloud->RawBody(data);
	CString response = cloud->send();
	if (response == "OK") {
		return this->query("DELETE FROM register; DELETE FROM problem; DELETE FROM answer; DELETE FROM lecture");
	}
	if (response == "FAIL")
		Error(E_WARNING, _T("�ϴ����ƶ˹������ڲ�����"));
	else
		Error(E_NOTICE, _T("�޷����ӵ��ƶ�"));
	return false;
}
bool LocalSto::initDbFile()
{
	if (!this->query("CREATE TABLE IF NOT EXISTS course ("
		"id INTEGER UNIQUE,"
		"course_id TEXT UNIQUE,"
		"name TEXT,"
		"lecture_count INTEGER)"))
		return false;
	if (!this->query("CREATE TABLE IF NOT EXISTS student ("
		"course INTEGER,"
		"student_id TEXT UNIQUE,"
		"numeric_id TEXT,"
		"name TEXT)"))
		return false;
	if (!this->query("CREATE TABLE IF NOT EXISTS product ("
		"product_id INTEGER UNIQUE,"
		"numeric_id TEXT)"))
		return false;
	if (!this->query("CREATE TABLE IF NOT EXISTS lecture ("
		"course TEXT,"
		"id INTEGER,"
		"begin_time INTEGER,"
		"end_time INTEGER)"))
		return false;
	if (!this->query("CREATE TABLE IF NOT EXISTS register ("
		"course TEXT,"
		"lecture INTEGER,"
		"product INTEGER,"
		"reg_time INTEGER)"))
		return false;
	if (!this->query("CREATE TABLE IF NOT EXISTS problem ("
		"course TEXT,"
		"lecture INTEGER,"
		"problem INTEGER,"
		"begin_time INTEGER,"
		"end_time INTEGER,"
		"correct_ans INTEGER)"))
		return false;
	if (!this->query("CREATE TABLE IF NOT EXISTS answer ("
		"course TEXT,"
		"lecture INTEGER,"
		"problem INTEGER,"
		"product INTEGER,"
		"ans INTEGER,"
		"ans_time INTEGER)"))
		return false;
	return true;
	/*
	HRSRC hRes = FindResource(0, MAKEINTRESOURCE(IDR_SQL1), L"SQL");
	if (NULL != hRes) {
		HGLOBAL hData = LoadResource(0, hRes);
		if(NULL != hData) {
			DWORD dataSize = SizeofResource(0, hRes);
			char* data = (char*)LockResource(hData);
			if (this->query(data))
				return true;
			Error(E_FATAL, L"�޷���ʼ�����ݿ�");
			return false;
		}
    }
	Error(E_FATAL, L"�Ҳ������ݿ��ʼ����Դ�ļ�");
	return false;
	*/
}

#define ASSERT_CHAR(str,c) do { \
	if (*(str)++ != (c)) { \
		Error(E_FATAL, L"���ƶ���������ʱ�ڲ�����"); \
		return false; \
	} \
} while(0)
/* @brief	���ƶ�����ѧ��������Ϣ
 * @return	�Ƿ����سɹ�
 */
bool LocalSto::syncFromCloud()
{
	CloudConn *cloud = new CloudConn("sync");
	char *response = (LPSTR)(LPCTSTR)cloud->send();
	if (response == NULL || *response == '\0') {
		Error(E_NOTICE, L"�޷����ӵ��ƶ�");
		return false;
	}
	response = this->loadDataInStr("course", "id,course_id,name,lecture_count", 3, response);
	ASSERT_CHAR(response, '\n');
	response = this->loadDataInStr("student", "course,student_id,numeric_id,name", 4, response);
	ASSERT_CHAR(response, '\n');
	response = this->loadDataInStr("product", "product_id,numeric_id", 2, response);
	return true;
}
/* @brief	�� MySQL �� LOAD DATA INFILE ���ƵĹ���
 * @param	table ����
 * @param	columns ���������ŷָ�
 * @param	column_count �е���Ŀ
 * @str		�����������
 * @return	ƥ��������һ���ַ���\0��\n��ָ��
 * @note	�����Ƿ����ɹ�������������е�����
 */
char* LocalSto::loadDataInStr(const char* table, const char* columns, const int column_count, char* str)
{
	this->squery("DELETE FROM %s", table); //��ձ�

	CString sql;
	sql.Format(L"INSERT INTO %s (%s) VALUES ", table, columns);
	bool first = true;
	while (*str && *str != '\n') {
		if (!first)
			sql += CString(",");
		else
			first = false;
		sql += CString("(");
		for (int i=0; i<column_count; i++) {
			if (i>0) {
				ASSERT_CHAR(str,'\t');
				sql += CString(",");
			}
			char *field;
			str = stripslashesForSpace(str, &field);
			sql += CString(field);
		}
		ASSERT_CHAR(str,'\n');
		sql += CString(")");
	}
	this->query((LPCSTR)(LPCTSTR)sql);
	return str;
}
/* @brief	�ñ������ݿ��ѧ��������Ϣ��ʼ���ڴ����ݽṹ
 * @param	m_List ѧ��������
 * @return	�Ƿ�ɹ�
 */
bool LocalSto::initStuNames(Students* s)
{
	sqlite3_stmt *stmt = NULL;
	CString sql;
	sql.Format(L"SELECT student_id,numeric_id,name FROM student WHERE course = %d", s->course);
	sqlite3_prepare(dbconn, (LPCSTR)(LPCTSTR)sql, -1, &stmt, (const char **)&errmsg);
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		CString student_id = (CString)sqlite3_column_text(stmt, 0);
		CString numeric_id = (CString)sqlite3_column_text(stmt, 1);
		CString name = (CString)sqlite3_column_text(stmt, 2);
		s->InfoList.Add(name, student_id, numeric_id);
	}
	sqlite3_finalize(stmt);
	return true;
}

//========== ������˽�к��� ==========
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
	return buf;
}
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
	vsprintf_s(sql, format, args);
	va_end(args);
	return LocalSto::query(sql);
}
bool LocalSto::query(const char* sql, int (*callback)(void*,int,char**,char**), void* argtocallback)
{
	return (SQLITE_OK == sqlite3_exec(dbconn, sql, callback, argtocallback, &errmsg));
}
char* LocalSto::selectFirst(const char* format, ...)
{
	char sql[SQL_MAXLEN];
	va_list args;
	va_start(args, format);
	vsprintf_s(sql, format, args);
	va_end(args);

	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare(dbconn, sql, -1, &stmt, (const char **)&errmsg);
	char* retval = NULL;
	if (SQLITE_ROW == sqlite3_step(stmt))
		retval = _strdup((char*)sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);
	return retval;
}

bool LocalSto::insert(const char* table, const char* fields, char* data1, ...)
{
	CString sql;
 	sql.Format(L"INSERT INTO %s (%s) VALUES (", table, fields);
	va_list args;
	va_start(args, data1);
	bool isFirst = true;
	while (char* data = va_arg(args, char*)) {
		if (isFirst)
			isFirst = false;
		else
			sql += CString(",");
		sql += CString("'");
		sql += CString(addslashes(data));
		sql += CString("'");
	}
	va_end(args);
	sql += CString(")");
	return this->query((LPCSTR)(LPCTSTR)sql);
}
bool LocalSto::insert(const char* table, const char* fields, long data1, ...)
{
	CString sql;
	sql.Format(L"INSERT INTO %s (%s) VALUES (", table, fields);
	va_list args;
	va_start(args, data1);
	bool isFirst = true;
	while (long data = va_arg(args, long)) {
		if (isFirst)
			isFirst = false;
		else
			sql += CString(",");
		CString data_int;
		data_int.Format(L"%ld", data);
		sql += data_int;
	}
	va_end(args);
	sql += CString(")");
	return this->query((LPCSTR)(LPCTSTR)sql);
}
