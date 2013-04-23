#include "stdafx.h"
#include "localsto.h"

static CString escape(CString str);
static CString addslashesForSpace(CString str);
static CString stripslashesForSpace(CString str, CString* rightstr);
static UINT toUINT(BYTE* s, int count);
static BYTE* toBytes(UINT num, int count);
static int CharacterCount(CString& csString_i, TCHAR sChar_i);
static CString UintToCString(UINT data);

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
	this->squery(_T("UPDATE lecture SET end_time=%ld WHERE course=%d AND id=%d"), time(NULL), course, lectureID);
	if (SQLITE_OK != sqlite3_close(dbconn)) {
		Error(E_WARNING, _T("�޷������ر����ݿ�"));
		error = 3;
	}
}
/* INTERNAL */
static int getcourse_callback(void* courses, int cols, char** values, char** fields)
{
	Course* newc = new Course((UINT)atoi(values[0]), CString(values[1]), CString(values[2]));
	((Courses*)courses)->add(newc);
	return 0;
}
/* @brief	��ȡ���а༶��Ϣ
 * @param	c ���ð༶��Ϣ�Ķ���
 * @return	�Ƿ��ȡ�ɹ�
 */
bool LocalSto::getCourses(Courses* c)
{
	return this->query(_T("SELECT id, name, info FROM course"), getcourse_callback, c);
}
/* @brief	����ѡ�еİ༶����ʼһ�ڿ�
 * @param	course_id �༶ID�ַ���
 * @return	�Ƿ��ʼ���ɹ�
 */
bool LocalSto::setCurCourse(UINT courseID)
{
	this->course = courseID;
	int lecture_count = atoi((CW2A)selectFirst(_T("SELECT lecture_count FROM course WHERE id=%d"), courseID));
	if (!squery(_T("UPDATE course SET lecture_count=lecture_count+1 WHERE course_id=%d"), courseID))
		goto error;
	if (!squery(_T("INSERT INTO lecture (course,id,begin_time) VALUES (%d,%d,%d)"), courseID, lecture_count+1, time(NULL)))
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
	if (!this->insert(_T("problem"), _T("course,lecture,problem,begin_time,end_time,correct_ans"),
		course, lectureID, s->QuesTotal, s->beginTime, currTime, s->CorAnswer))
		return false;
	Stu* stu = s->head;
	bool success = true;
	while ((stu = stu->next) != NULL) {
		if (!this->insert(_T("answer"), _T("course,lecture,problem,product,answer,ans_time,mark"),
			course, lectureID, s->QuesTotal, stu->ProductId, stu->Ans, stu->AnsTime, stu->mark))
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
	return this->squery(_T("UPDATE problem SET correct_ans=%d WHERE course=%d AND lecture=%d AND problem=%d"),
		course, s->CorAnswer, lectureID, s->QuesTotal);
}
/* @brief	ѧ��ǩ��
 * @param	ProductId ��ƷID
 * @return	�Ƿ񱣴�ɹ�
 */
bool LocalSto::stuSignIn(UINT ProductId)
{
	return this->insert(_T("register"), _T("course,lecture,product,reg_time"),
		course, lectureID, ProductId, time(NULL));
}
/* @brief	����ѧ�����õ�ѧ��
 * @param	NumericId ����ѧ��
 * @param	ProductId ��ƷID
 * @return	�Ƿ񱣴�ɹ�
 */
bool LocalSto::setNumericId(CString NumericId, UINT ProductId)
{
	return this->squery(_T("REPLACE INTO product (id,numeric_id) VALUES (%u,%s)"),
		ProductId, escape(NumericId));
}
/* @brief	�����ݿ��ѯ�����ʾ���ַ���
 * @param	sql ���ݿ��ѯ
 * @return	��ѯ�ṹ
 */
CString LocalSto::rowsToStr(CString sql)
{
	CString str = _T("");
	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare(dbconn, (CW2A)sql, -1, &stmt, (const char**)&errmsg);
	int col_count = sqlite3_column_count(stmt);
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		for (int i=0; i<col_count; i++) {
			if (i>0)
				str += _T("\t");
			str += addslashesForSpace(CString((char *)sqlite3_column_text(stmt, i)));
		}
		str += _T("\n");
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
	if (!insert(_T("course"), _T("name,info"), course->name, course->info)) {
		error = 5;
		Error(E_FATAL, _T("�޷�����γ̵��������ݿ�\n������Ϣ��\n") + CString(errmsg));
		return false;
	}
	sqlite3_int64 course_id = sqlite3_last_insert_rowid(dbconn);
	course->id = (UINT)course_id;
	CString data;
	data.Format(_T("%u\n"), course_id);
	data += course->name + _T("\n") + course->info + _T("\n");
	CloudConn *cloud = new CloudConn(_T("add_course"));
	cloud->RawBody(data);
	CString response = cloud->send(StationID, StationToken);
	if (response != _T("OK")) {
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
	data = this->rowsToStr(_T("SELECT id,lecture_count FROM course"));
	data += "\n";
	data += this->rowsToStr(_T("SELECT id,numeric_id FROM product"));
	data += "\n";
	data += this->rowsToStr(_T("SELECT course,id,begin_time,end_time FROM lecture"));
	data += "\n";
	data += this->rowsToStr(_T("SELECT course,product,lecture,reg_time FROM register"));
	data += "\n";
	data += this->rowsToStr(_T("SELECT course,lecture,problem,begin_time,end_time,corrent_ans FROM problem"));
	data += "\n";
	data += this->rowsToStr(_T("SELECT course,lecture,problem,product,ans,ans_time,mark FROM answer"));

	CloudConn *cloud = new CloudConn(_T("upload"));
	cloud->RawBody(data);
	CString response = cloud->send(StationID, StationToken);
	if (response == _T("OK")) {
		if (this->query(_T("DELETE FROM register; DELETE FROM problem; DELETE FROM answer; DELETE FROM lecture")))
			return true;
		error = 6;
		Error(E_WARNING, _T("�޷�������ݿ��е���Ч����"));
	}
	else if (response == _T("")) {
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
	if (!this->query(_T("CREATE TABLE IF NOT EXISTS course ( \
		id INTEGER PRIMARY KEY, \
		course_id TEXT, \
		name TEXT, \
		lecture_count INTEGER, \
		info TEXT \
		)")))
		return false;
	if (!this->query(_T("CREATE TABLE IF NOT EXISTS student ( \
		course INTEGER, \
		student_id TEXT, \
		numeric_id TEXT UNIQUE, \
		name TEXT \
		)")))
		return false;
	if (!this->query(_T("CREATE TABLE IF NOT EXISTS product ( \
		id INTEGER UNIQUE, \
		numeric_id TEXT \
		)")))
		return false;
	if (!this->query(_T("CREATE TABLE IF NOT EXISTS lecture ( \
		course INTEGER, \
		id INTEGER, \
		begin_time INTEGER, \
		end_time INTEGER \
		)")))
		return false;
	if (!this->query(_T("CREATE TABLE IF NOT EXISTS register ( \
		course INTEGER, \
		lecture INTEGER, \
		product INTEGER, \
		reg_time INTEGER \
		)")))
		return false;
	if (!this->query(_T("CREATE TABLE IF NOT EXISTS problem ( \
		course INTEGER, \
		lecture INTEGER, \
		problem INTEGER, \
		begin_time INTEGER, \
		end_time INTEGER, \
		correct_ans INTEGER \
		)")))
		return false;
	if (!this->query(_T("CREATE TABLE IF NOT EXISTS answer ( \
		course INTEGER, \
		lecture INTEGER, \
		problem INTEGER, \
		product INTEGER, \
		ans INTEGER, \
		ans_time INTEGER, \
		mark INTEGER \
		)")))
		return false;
	return true;
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
	CloudConn *cloud = new CloudConn(_T("sync"));
	CString response = cloud->send(StationID, StationToken);
	if (response == _T("")) {
		error = 10;
		Error(E_NOTICE, _T("�޷����ӵ��ƶ�"));
		return false;
	}
	response = this->loadDataInStr(_T("course"), _T("id,course_id,name,lecture_count,info"), 5, response);
	ASSERT_CHAR(response, '\n');
	response = this->loadDataInStr(_T("student"), _T("course,student_id,numeric_id,name"), 4, response);
	ASSERT_CHAR(response, '\n');
	response = this->loadDataInStr(_T("product"), _T("id,numeric_id"), 2, response);
	return true;
error:
	error = 9;
	Error(E_FATAL, _T("���ƶ����ص����ݸ�ʽ����"));
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
	this->squery(_T("DELETE FROM %s"), table); //��ձ�

	CString sql;
	sql.Format(_T("INSERT INTO %s (%s) VALUES "), table, columns);
	bool firstLine = true;
	while (str.GetLength() > 0) {
		if (!firstLine)
			ASSERT_CHAR(str, '\n');
		if (str.GetAt(0) == '\n') // the end of this section
			break;
		if (!firstLine)
			sql += _T(",");
		if (firstLine)
			firstLine = false;
		sql += _T("(");
		for (int i=0; i<column_count; i++) {
			if (i>0) {
				ASSERT_CHAR(str, '\t');
				sql += _T(",");
			}
			CString *rightstr = new CString();
			sql += escape(stripslashesForSpace(str, rightstr));
			str = *rightstr;
		}
		sql += _T(")");
	}
	if (firstLine == true) // û������
		return str;
	if (!this->query(sql)) {
		error = 11;
		Error(E_WARNING, _T("���ƶ�ͬ������ʱ���ݿ����\n������Ϣ��\n") + CString(errmsg) + _T("\n") + sql);
	}
	return str;
error:
	error = 9;
	Error(E_FATAL, _T("���ƶ����ص����ݸ�ʽ����"));
	return _T("");
}
/* @brief	�ñ������ݿ��ѧ��������Ϣ��ʼ���ڴ����ݽṹ
 * @param	m_List ѧ��������
 * @return	�Ƿ�ɹ�
 */
bool LocalSto::initStuNames(Students* s)
{
	sqlite3_stmt *stmt = NULL;
	CString sql;
	sql.Format(_T("SELECT student_id,numeric_id,name FROM student WHERE course = %d"), s->course);
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
	str.Replace(_T("'"), _T("\\'"));
	return _T("'") + str + _T("'");
}
static CString addslashesForSpace(CString str)
{
	str.Replace(_T("\\"), _T("\\\\"));
	str.Replace(_T("\t"), _T("\\\t"));
	str.Replace(_T("\n"), _T("\\\n"));
	return str;
}
static CString stripslashesForSpace(CString str, CString* right)
{
	CString left = _T("");
	while (str.GetLength() > 0) {
		int n = str.FindOneOf(_T("\t\n"));
		
		if (n == -1) // not found
			break;
		if (n == 0) {
			str = str.Right(str.GetLength());
			break;
		}
		if (str.GetAt(n-1) == '\\') { // this is an escaped \t or \n
			left += str.Left(n-1);
			left += str.GetAt(n);
			str = str.Right(str.GetLength() - n - 1);
			continue;
		}
		else { // \t or \n as separator
			left += str.Left(n);
			str = str.Right(str.GetLength() - n);
			break;
		}
	}
	left.Replace(_T("\\\\"), _T("\\"));
	*right = str;
	return left;
}
bool LocalSto::getAll(CString table, int (*callback)(void*,int,char**,char**))
{
	CString sql;
	sql.Format(_T("SELECT * FROM %s"), table);
	return this->query(sql, callback, NULL);
}
bool LocalSto::select(CString table, CString field, CString value, int (*callback)(void*,int,char**,char**))
{
	CString sql;
	sql.Format(_T("SELECT * FROM %s WHERE %s=%s"), table, field, escape(value));
	return this->query(sql, callback, NULL);
}
bool LocalSto::update(CString table, CString searchField, CString searchValue, CString updateField, CString updateValue)
{
	return this->squery(_T("UPDATE %s SET %s=%s WHERE %s=%s"),
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

#define __GENERIC_INSERT(Type, DataHandler) \
bool LocalSto::insert(CString table, CString fields, Type data1, ...) \
{ \
	int commaCount = CharacterCount(fields, ','); \
	CString sql; \
 	sql.Format(_T("INSERT INTO %s (%s) VALUES ("), table, fields); \
	va_list args; \
	va_start(args, data1); \
	sql += DataHandler(data1); \
	for (int i=0; i<commaCount; i++) { \
		Type d = va_arg(args, Type); \
		sql += _T(","); \
		sql += DataHandler(d); \
	} \
	va_end(args); \
	sql += _T(")"); \
	return this->query(sql); \
}
__GENERIC_INSERT(UINT, UintToCString)
__GENERIC_INSERT(CString, escape)

static int CharacterCount(CString& csString_i, TCHAR sChar_i)
{
	if( csString_i.IsEmpty())
		return 0;
	int nFind = -1;
	int nCount = 0;
	while( -1 != ( nFind = csString_i.Find( sChar_i, nFind + 1 )))
		nCount++;
	return nCount;
}
static CString UintToCString(UINT data)
{
	CString data_int;
	data_int.Format(_T("%u"), data);
	return data_int;
}