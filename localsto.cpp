#include "stdafx.h"
#include "localsto.h"

static CString addslashes(CString str);
static CString addslashesForSpace(CString str);
static CString stripslashesForSpace(CString str, CString* rightstr);
static UINT toUINT(BYTE* s, int count);
static BYTE* toBytes(UINT num, int count);

/* @brief	���ش洢�๹�캯��
 * @param	StationID ID
 */
LocalSto::LocalSto(UINT StationID, CString StationToken)
{
	this->error = 0;
	this->StationID = StationID;
	this->StationToken = StationToken;
	if (SQLITE_OK != sqlite3_open("test.db", &this->dbconn)) {
		error = 1;
		Error(E_FATAL, _T("�޷������ݿ�"));
		return;
	}
	if (!this->initDbFile()) {
		error = 2;
		Error(E_FATAL, _T("�޷���ʼ�����ݿ�ṹ"));
		return;
	}
	this->syncFromCloud();
}
LocalSto::~LocalSto()
{
	this->squery(L"UPDATE lecture SET end_time=%ld WHERE course='%s' AND id=%d", time(NULL), course, lectureID);
	if (SQLITE_OK != sqlite3_close(dbconn)) {
		Error(E_WARNING, _T("�޷������ر����ݿ�"));
		error = 3;
	}
}
/* INTERNAL */
static int getcourse_callback(void* courses, int cols, char** values, char** fields)
{
	Course* newc = new Course(atoi(values[0]), values[1], values[2]);
	((Courses*)courses)->add(newc);
	return 0;
}
/* @brief	��ȡ���а༶��Ϣ
 * @param	c ���ð༶��Ϣ�Ķ���
 * @return	�Ƿ��ȡ�ɹ�
 */
bool LocalSto::getCourses(Courses* c)
{
	return this->query(CString("SELECT id, name, info FROM course"), getcourse_callback, c);
}
/* @brief	����ѡ�еİ༶����ʼһ�ڿ�
 * @param	course_id �༶ID�ַ���
 * @return	�Ƿ��ʼ���ɹ�
 */
bool LocalSto::setCurCourse(UINT courseID)
{
	this->course = courseID;
	int lecture_count = atoi((CW2A)selectFirst(CString("SELECT lecture_count FROM course WHERE id=%d"), courseID));
	if (!squery(CString("UPDATE course SET lecture_count=lecture_count+1 WHERE course_id=%d"), courseID))
		goto error;
	if (!squery(CString("INSERT INTO lecture (course,id,begin_time) VALUES (%d,%d,%ld)"), courseID, lecture_count+1, time(NULL)))
		goto error;
	this->lectureID = (UINT)sqlite3_last_insert_rowid(this->dbconn);
	return true;
error:
	error = 4;
	Error(E_FATAL, _T("���ݿ��ʼ��������Ϣʧ��"));
	return false;
}
/* @brief	����һ����Ĵ�����Ϣ
 * @param	s ���ö���
 * @return	�Ƿ񱣴�ɹ�
 */
bool LocalSto::saveAnswers(Students *s)
{
	UINT currTime = (UINT)time(NULL);
	if (!this->insert(CString("problem"), CString("course,lecture,problem,begin_time,end_time,correct_ans"),
		course, lectureID, s->QuesTotal, s->beginTime, currTime, s->CorAnswer))
		return false;
	Stu* stu = s->head;
	bool success = true;
	while ((stu = stu->next) != NULL) {
		if (!this->insert(CString("answer"), CString("course,lecture,problem,product,answer,ans_time"),
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
	return this->squery(CString("UPDATE problem SET correct_ans=%d WHERE course=%d AND lecture=%d AND problem=%d"),
		course, s->CorAnswer, lectureID, s->QuesTotal);
}
/* @brief	ѧ��ǩ��
 * @param	ProductId ��ƷID
 * @return	�Ƿ񱣴�ɹ�
 */
bool LocalSto::stuSignIn(UINT ProductId)
{
	return this->insert(CString("register"), CString("course,lecture,product,reg_time"),
		course, lectureID, ProductId, time(NULL));
}
/* @brief	����ѧ�����õ�ѧ��
 * @param	NumericId ����ѧ��
 * @param	ProductId ��ƷID
 * @return	�Ƿ񱣴�ɹ�
 */
bool LocalSto::setNumericId(CString NumericId, UINT ProductId)
{
	return this->squery(CString("REPLACE INTO product (id,numeric_id) VALUES ('%ld','%s')"),
		ProductId, NumericId);
}
/* @brief	�����ݿ��ѯ�����ʾ���ַ���
 * @param	sql ���ݿ��ѯ
 * @return	��ѯ�ṹ
 */
CString LocalSto::rowsToStr(CString sql)
{
	CString str = "";
	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare(dbconn, (CW2A)sql, -1, &stmt, (const char**)&errmsg);
	int col_count = sqlite3_column_count(stmt);
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		for (int i=0; i<col_count; i++) {
			if (i>0)
				str += CString(L"\t");
			str += addslashesForSpace(CString((char *)sqlite3_column_text(stmt, i)));
		}
		str += CString(L"\n");
	}
	sqlite3_finalize(stmt);
	return str;
}
/* @brief	��ӿγ�
 * @param	name �γ�����
 * @param	info �γ�����
 */
bool LocalSto::addCourse(Course* course)
{
	if (!insert(CString("course"), CString("name,info"), course->name, course->info)) {
		error = 5;
		Error(E_FATAL, _T("�޷�����γ̵��������ݿ�"));
		return false;
	}
	sqlite3_int64 course_id = sqlite3_last_insert_rowid(dbconn);
	course->id = (UINT)course_id;
	CString data;
	data.Format(L"%u\n", course_id);
	data += course->name + "\n" + course->info + "\n";
	CloudConn *cloud = new CloudConn(CString("add_course"));
	cloud->RawBody(data);
	CString response = cloud->send(StationID, StationToken);
	if (response != "OK") {
		error = 6;
		Error(E_NOTICE, _T("�޷�����γ̵�������"));
		return false;
	}
	return true;
}
/* @brief	���������ݿ��ѧ��ǩ����Ϣ�ʹ�����Ϣ�ϴ����ƶ�
 * @return	�Ƿ��ϴ��ɹ�
 */
bool LocalSto::uploadToCloud()
{
	CString data;
	data = this->rowsToStr(CString("SELECT id,lecture_count FROM course"));
	data += "\n";
	data += this->rowsToStr(CString("SELECT id,numeric_id FROM product"));
	data += "\n";
	data += this->rowsToStr(CString("SELECT course,id,begin_time,end_time FROM lecture"));
	data += "\n";
	data += this->rowsToStr(CString("SELECT course,product,lecture,reg_time FROM register"));
	data += "\n";
	data += this->rowsToStr(CString("SELECT course,lecture,problem,begin_time,end_time,corrent_ans FROM problem"));
	data += "\n";
	data += this->rowsToStr(CString("SELECT course,lecture,problem,product,ans,ans_time FROM answer"));

	CloudConn *cloud = new CloudConn(CString("upload"));
	cloud->RawBody(data);
	CString response = cloud->send(StationID, StationToken);
	if (response == "OK") {
		if (this->query(CString("DELETE FROM register; DELETE FROM problem; DELETE FROM answer; DELETE FROM lecture")))
			return true;
		error = 6;
		Error(E_WARNING, _T("�޷�������ݿ��е���Ч����"));
	}
	else if (response == "") {
		error = 7;
		Error(E_NOTICE, _T("�޷����ӵ��ƶ�"));
	}
	else {
		error = 8;
		Error(E_WARNING, _T("�ϴ����ƶ˹������ڲ�����"));
	}
	return false;
}
bool LocalSto::initDbFile()
{
	if (!this->query(CString("CREATE TABLE IF NOT EXISTS course ("
		"id INTEGER PRIMARY KEY,"
		"course_id TEXT UNIQUE,"
		"name TEXT,"
		"lecture_count INTEGER,"
		"info TEXT)")))
		return false;
	if (!this->query(CString("CREATE TABLE IF NOT EXISTS student ("
		"course INTEGER,"
		"student_id TEXT UNIQUE,"
		"numeric_id TEXT,"
		"name TEXT)")))
		return false;
	if (!this->query(CString("CREATE TABLE IF NOT EXISTS product ("
		"id INTEGER UNIQUE,"
		"numeric_id TEXT)")))
		return false;
	if (!this->query(CString("CREATE TABLE IF NOT EXISTS lecture ("
		"course INTEGER,"
		"id INTEGER,"
		"begin_time INTEGER,"
		"end_time INTEGER)")))
		return false;
	if (!this->query(CString("CREATE TABLE IF NOT EXISTS register ("
		"course INTEGER,"
		"lecture INTEGER,"
		"product INTEGER,"
		"reg_time INTEGER)")))
		return false;
	if (!this->query(CString("CREATE TABLE IF NOT EXISTS problem ("
		"course INTEGER,"
		"lecture INTEGER,"
		"problem INTEGER,"
		"begin_time INTEGER,"
		"end_time INTEGER,"
		"correct_ans INTEGER)")))
		return false;
	if (!this->query(CString("CREATE TABLE IF NOT EXISTS answer ("
		"course INTEGER,"
		"lecture INTEGER,"
		"problem INTEGER,"
		"product INTEGER,"
		"ans INTEGER,"
		"ans_time INTEGER)")))
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

#define ASSERT_CHAR(s,c) do { \
	if (s.GetAt(0) != (c)) { \
		goto error; \
	} \
	s = s.Right(s.GetLength() - 1); \
} while(0)

/* @brief	���ƶ�����ѧ��������Ϣ
 * @return	�Ƿ����سɹ�
 */
bool LocalSto::syncFromCloud()
{
	CloudConn *cloud = new CloudConn(CString("sync"));
	CString response = cloud->send(StationID, StationToken);
	if (response == "") {
		error = 10;
		Error(E_NOTICE, L"�޷����ӵ��ƶ�");
		return false;
	}
	response = this->loadDataInStr(CString("course"), CString("id,course_id,name,lecture_count,info"), 5, response);
	ASSERT_CHAR(response, '\n');
	response = this->loadDataInStr(CString("student"), CString("course,student_id,numeric_id,name"), 4, response);
	ASSERT_CHAR(response, '\n');
	response = this->loadDataInStr(CString("product"), CString("id,numeric_id"), 2, response);
	return true;
error:
	error = 9;
	Error(E_FATAL, L"���ƶ����ص����ݸ�ʽ����");
	return false;
}

/* @brief	�� MySQL �� LOAD DATA INFILE ���ƵĹ���
 * @param	table ����
 * @param	columns ���������ŷָ�
 * @param	column_count �е���Ŀ
 * @str		�����������
 * @return	ƥ�����ʣ�ಿ��
 * @note	�����Ƿ����ɹ�������������е�����
 */
CString LocalSto::loadDataInStr(CString table, CString columns, const int column_count, CString str)
{
	this->squery(CString("DELETE FROM %s"), table); //��ձ�

	CString sql;
	sql.Format(L"INSERT INTO %s (%s) VALUES ", table, columns);
	bool first = true;
	while (str.GetLength() > 0) {
		if (!first)
			sql += CString(",");
		else
			first = false;
		sql += CString("(");
		for (int i=0; i<column_count; i++) {
			if (i>0) {
				ASSERT_CHAR(str, '\t');
				sql += CString(",");
			}
			CString *rightstr = new CString();
			sql += stripslashesForSpace(str, rightstr);
			str = *rightstr;
		}
		ASSERT_CHAR(str, '\n');
		sql += CString(")");
	}
	this->query(sql);
	return str;
error:
	error = 9;
	Error(E_FATAL, L"���ƶ����ص����ݸ�ʽ����");
	return "";
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
	sqlite3_prepare(dbconn, (CW2A)sql, -1, &stmt, (const char **)&errmsg);
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
static UINT toUINT(BYTE* s, int count)
{
	UINT l = 0;
	for (int i=0; i<count; i++)
		l = (l<<8) + s[i];
	return l;
}
static BYTE* toBytes(UINT num, int count)
{
	BYTE *buf = (BYTE*)malloc(count);
	for (int i=count-1; i>=0; i--) {
		buf[i] = num & 0xFF;
		num>>=8;
	}
	return buf;
}
static CString escape(CString str)
{
	str.Replace(L"'", L"\\'");
	return "'" + str + "'";
}
static CString addslashesForSpace(CString str)
{
	str.Replace(L"\\", L"\\\\");
	str.Replace(L"\t", L"\\\t");
	str.Replace(L"\n", L"\\\n");
	return str;
}
static CString stripslashesForSpace(CString str, CString* right)
{
	CString left = "";
	while (str.GetLength() > 0) {
		int n = str.FindOneOf(L"\t\n");
		if (n == -1) // not found
			break;
		if (n == 0) {
			str = str.Right(str.GetLength() - 1);
			break;
		}
		if (str.GetAt(n-1) == '\\') {
			left += str.Left(n-1);
			left += str.GetAt(n);
			str = str.Right(str.GetLength() - n - 1);
			continue;
		}
		else break;
	}
	left.Replace(L"\\\\", L"\\");
	*right = str;
	return left;
}
bool LocalSto::getAll(CString table, int (*callback)(void*,int,char**,char**))
{
	CString sql;
	sql.Format(L"SELECT * FROM %s", table);
	return this->query(sql, callback, NULL);
}
bool LocalSto::select(CString table, CString field, CString value, int (*callback)(void*,int,char**,char**))
{
	CString sql;
	sql.Format(L"SELECT * FROM %s WHERE %s=%s", table, field, escape(value));
	return this->query(sql, callback, NULL);
}
bool LocalSto::update(CString table, CString searchField, CString searchValue, CString updateField, CString updateValue)
{
	return this->squery(CString("UPDATE %s SET %s=%s WHERE %s=%s"),
		table,
		updateField, escape(updateValue), 
		searchField, escape(searchValue));
}
bool LocalSto::query(CString sql)
{
	return (SQLITE_OK == sqlite3_exec(dbconn, (CW2A)sql, NULL, 0, &errmsg));
}
bool LocalSto::squery(CString format, ...)
{
	CString sql;
	va_list args;
	va_start(args, format);
	sql.FormatV(format, args);
	va_end(args);
	return LocalSto::query(sql);
}
bool LocalSto::query(CString sql, int (*callback)(void*,int,char**,char**), void* argtocallback)
{
	return (SQLITE_OK == sqlite3_exec(dbconn, (CW2A)sql, callback, argtocallback, &errmsg));
}
CString LocalSto::selectFirst(CString format, ...)
{
	CString sql;
	va_list args;
	va_start(args, format);
	sql.FormatV(format, args);
	va_end(args);

	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare(dbconn, (CW2A)sql, -1, &stmt, (const char **)&errmsg);
	char* retval = NULL;
	if (SQLITE_ROW == sqlite3_step(stmt))
		retval = _strdup((char*)sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);
	return CString(retval);
}

bool LocalSto::insert(CString table, CString fields, CString data1, ...)
{
	CString sql;
 	sql.Format(L"INSERT INTO %s (%s) VALUES (", table, fields);
	va_list args;
	va_start(args, data1);
	bool isFirst = true;
	while (CString data = va_arg(args, CString)) {
		if (isFirst)
			isFirst = false;
		else
			sql += CString(",");
		sql += CString("'");
		sql += escape(data);
		sql += CString("'");
	}
	va_end(args);
	sql += CString(")");
	return this->query(sql);
}
bool LocalSto::insert(CString table, CString fields, UINT data1, ...)
{
	CString sql;
	sql.Format(L"INSERT INTO %s (%s) VALUES (", table, fields);
	va_list args;
	va_start(args, data1);
	bool isFirst = true;
	while (UINT data = va_arg(args, UINT)) {
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
	return this->query(sql);
}
