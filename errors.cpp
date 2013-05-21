#include "stdafx.h"
#include "errors.h"

CString error_msgs[] = {
	_T("无法打开数据库"),
	_T("无法初始化数据库结构"),
	_T("无法正常关闭数据库"),
	_T("上节课还没有结束，这节课就开始了？"),
	_T("上节课没有正常结束，将继续进行上一节课"),
	_T("数据库初始化课堂信息失败"),
	_T("在没有开始上课的时候结束上课"),
	_T("结束上课的数据库查询失败"),
	_T("保存题目信息失败"),
	_T("保存学生答案失败"),
	_T("无法保存课程到本地数据库"),
	_T("无法保存课程到服务器"),
	_T("无法清空数据库中的已发送数据"),
	_T("无法连接到云端"),
	_T("上传到云端过程中内部错误"),
	_T("从云端下载的数据格式错误"),
	_T("从云端同步数据时数据库错误"),
	_T("连接云端的请求被拒绝"),
};

void Error(int level, int errorno)
{
	if (errorno == E_FATAL || errorno == E_WARNING) {
		TRACE(error_msgs[errorno]);
	}
	if (errorno == E_FATAL) {
		AfxMessageBox(error_msgs[errorno]);
		exit(1);
	}
}