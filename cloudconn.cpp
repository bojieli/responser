#include "stdafx.h"
#include "cloudconn.h"

CloudConn::CloudConn(CString path)
{
	try {
		session = new CInternetSession(L"responser");
		session->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 1000 * 3);
		session->SetOption(INTERNET_OPTION_CONNECT_BACKOFF, 1000);
		session->SetOption(INTERNET_OPTION_CONNECT_RETRIES, 2);
		conn = session->GetHttpConnection(L"api.gewuit.com");
		file = conn->OpenRequest(CHttpConnection::HTTP_VERB_POST,
								 CString(L"/responser/") + path,
								 NULL,
								 1,
								 NULL,
								 TEXT("HTTP/1.1"),
								 INTERNET_FLAG_RELOAD);
	} catch(CInternetException * m_pException) {
		m_pException->Delete();
		Error(E_WARNING, L"无法连接到服务器");
	}
}
CloudConn::~CloudConn()
{
	file->Close();
    conn->Close();
    session->Close();
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
void CloudConn::RawBody(CString str)
{
	postData = str;
}
CString CloudConn::send(UINT StationID, CString StationToken)
{
	DWORD dwRet;
	try {
		CString header;
		header.Format(L"User-Agent: " USER_AGENT L"\r\nStation-ID: %u\r\nStation-Token: %s\r\n", StationID, StationToken);
		file->SendRequest(header, header.GetLength(), (LPVOID)(LPCTSTR)postData, postData.GetLength());
	} catch(CInternetException * m_pException) {
		m_pException->Delete();
		Error(E_WARNING, L"无法连接到服务器");
		return "";
	}
	file->QueryInfoStatusCode(dwRet);
    if(dwRet != HTTP_STATUS_OK) {
		Error(E_WARNING, L"与云端间的数据通信错误");
		return "";
    }
	CString response = "";
	UINT recvsize;
	char buf[1025] = {0};
    while (recvsize = file->Read(buf, 1024)) {
		buf[recvsize] = '\0';
        response += CString(buf);
	}
	return response;
}