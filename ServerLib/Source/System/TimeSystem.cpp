
#include "../../Include/System/TimeSystem.h"

#ifdef _WIN_
#include "../../Include/Windows/WindowsTimeSystem.h"
#else
#endif

//	获取格林尼治时间（单位：秒）
double GetGMTTime()
{
	double dRet = 0.0;
#ifdef _WIN_
	dRet = Windows_GetGMTTime();
#else
#endif

	return dRet;
}

//	获取格林尼治时间时间戳（单位：毫秒）
double GetGMTTimeStamp()
{
	double dRet = 0.0;
#ifdef _WIN_
	dRet = Windows_GetGMTTimeStamp();
#else
#endif
	return dRet;
}

//	获取时间戳（单位：毫秒）
uint32 GetTimeStamp()
{
	uint32 uRet = 0;
#ifdef _WIN_
	uRet = Windows_GetTimeStamp();
#else
#endif
	return uRet;
}

//	获取日期
bool GetCurTimeString(char* strTime, uint32 uStrCout, const char* strParam, bool bNeedDayOfWeek)
{
	bool bRet = false;
#ifdef _WIN_
	bRet = Windows_GetCurTimeString(strTime, uStrCout, strParam, bNeedDayOfWeek);
#else
#endif
	return bRet;
}





