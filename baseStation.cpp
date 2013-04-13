#include "stdafx.h"
#include "baseStation.h"

UINT BaseStation::ID() {
	return 0xdeadbeef;
}
CString BaseStation::token() {
	return CString(L"helloworld");
}