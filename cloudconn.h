#pragma once

#include "stdafx.h"
#include <afxinet.h>

#define PROTOCOL_VERSION "1.0"
#define USER_AGENT "responser/" PROTOCOL_VERSION

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
	CString send(UINT StationId, CString StationToken);
};