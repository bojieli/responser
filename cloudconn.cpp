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
		Error(E_WARNING, L"�޷����ӵ�������");
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
		Error(E_WARNING, L"�޷����ӵ�������");
		return "";
	}
	file->QueryInfoStatusCode(dwRet);
    if(dwRet != HTTP_STATUS_OK) {
		Error(E_WARNING, L"���ƶ˼������ͨ�Ŵ���");
		return "";
    }

#define BUF_SIZE 1024
	char* response = NULL;
	UINT totalsize = 0;
	UINT recvsize;
	char buf[BUF_SIZE+1];
    while (recvsize = file->Read(buf, BUF_SIZE)) {
		response = (char*)realloc(response, totalsize + recvsize);
		memcpy(response + totalsize, buf, recvsize);
		totalsize += recvsize;
	}
	response[totalsize] = '\0';
	int w_response_len = MultiByteToWideChar(CP_UTF8, 0, response, -1, NULL, 0);
	wchar_t* w_response = new wchar_t[w_response_len+1];
	memset(w_response, 0, (w_response_len+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, response, -1, w_response, w_response_len);
	return CString(w_response);
}