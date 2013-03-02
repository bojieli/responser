#include "stdafx.h"
#include "http.h"

CloudConn::CloudConn(const char* path)
{
	session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 1000 * 3);
    session.SetOption(INTERNET_OPTION_CONNECT_BACKOFF, 1000);
    session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 2);
	conn = session.GetHttpConnection((LPCTSTR)"api.gewuit.com");
	file = conn->OpenRequest(CHttpConnection::HTTP_VERB_POST,
                             CString("/responser/") + path,
                             NULL,
                             1,
                             NULL,
                             TEXT("HTTP/1.1"),
                             INTERNET_FLAG_RELOAD);
}
CloudConn::~CloudConn()
{
	file->Close();
    conn->Close();
    session.Close();
    delete file;
    delete conn;
}
void CloudConn::SetBody(CString key, CString value)
{
	if (postData.GetLength() > 0)
		postData += CString(_T('&'));
	postData += key;
	postData += CString(_T('='));
	postData += value;
}
CString CloudConn::send()
{
    file->SendRequest(NULL, 0, postData, postData.GetLength());
    DWORD dwRet;
    file->QueryInfoStatusCode(dwRet);
    
    if(dwRet != HTTP_STATUS_OK) {
		Error(E_WARNING, "与云端间的数据通信错误");
		return NULL;
    }
	CString response;
    CString tmp;
    while (file->ReadString(tmp))
        response += tmp;
	return response;
}