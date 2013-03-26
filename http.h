#pragma once

#include "stdafx.h"
#include <afxinet.h>

class CloudConn
{
protected:
	CInternetSession *session;
	CHttpConnection *conn;
	CHttpFile *file;
	CString postData;
public:
	CloudConn(const char* path);
	~CloudConn();
	void SetBody(CString key, CString value);
	void RawBody(CString str);
	CString send();
};