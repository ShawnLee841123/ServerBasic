

#ifdef _WIN_

#include "../../Include/Windows/WindowsTimeSystem.h"
#include <windows.h>
#include <stdio.h>
double Windows_GetGMTTime()
{
	FILETIME ft;
	memset(&ft, 0, sizeof(ft));
	GetSystemTimeAsFileTime(&ft);
	double dTime = (double)(*((uint64*)&ft));
	dTime = dTime / 10000 / 1000 - 1164473600;
	return dTime;
}

double Windows_GetGMTTimeStamp()
{
	double dTimeStamp = Windows_GetGMTTime() * 1000.0;
	return dTimeStamp;
}

uint32 Windows_GetTimeStamp()
{
	return Windows_GetGMTTimeStamp();
}

bool Windows_GetCurTimeString(char* strTime, uint32 uStrCout, const char* strParam, bool bNeedDayOfWeek)
{
	if (nullptr == strTime)
		return false;

	if (uStrCout <= 0)
		return false;

	SYSTEMTIME st;
	GetLocalTime(&st);

	//	年
	char strYear[16] = { 0 };
	sprintf(strYear, "%d", st.wYear);

	//	月
	char strMonth[16] = { 0 };
	if ((st.wMonth / 10) > 0)
		sprintf(strMonth, "%d", st.wMonth);
	else
		sprintf(strMonth, "0%d", st.wMonth);

	//	日
	char strDay[16] = { 0 };
	if ((st.wDay / 10) > 0)
		sprintf(strDay, "%d", st.wDay);
	else
		sprintf(strDay, "0%d", st.wDay);

	//	周
	char strDayOfWeek[16] = { 0 };
	sprintf(strDayOfWeek, "%d", st.wDayOfWeek);

	//	时
	char strHour[16] = { 0 };
	if ((st.wHour / 10) > 0)
		sprintf(strHour, "%d", st.wHour);
	else
		sprintf(strHour, "0%d", st.wHour);

	//	分
	char strMinute[16] = { 0 };
	if ((st.wMinute / 10) > 0)
		sprintf(strMinute, "%d", st.wMinute);
	else
		sprintf(strMinute, "0%d", st.wMinute);

	//	秒
	char strSecond[16] = { 0 };
	if ((st.wSecond / 10) > 0)
		sprintf(strSecond, "%d", st.wSecond);
	else
		sprintf(strSecond, "0%d", st.wSecond);

	//	毫秒
	char strMillonSec[16] = { 0 };
	if ((st.wMilliseconds / 100) > 0)
		sprintf(strMillonSec, "%d", st.wMilliseconds);
	else
	{
		if (st.wMilliseconds / 10 > 0)
			sprintf(strMillonSec, "0%d", st.wMilliseconds);
		else
			sprintf(strMillonSec, "00%d", st.wMilliseconds);
	}


	if (bNeedDayOfWeek)
		sprintf(strTime, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", strYear, strParam, strMonth, strParam, strDay, strParam, strDayOfWeek, strParam, strHour, strParam, strMinute, strParam, strSecond, strParam, strMillonSec);
	else
		sprintf(strTime, "%s%s%s%s%s%s%s%s%s%s%s%s%s", strYear, strParam, strMonth, strParam, strDay, strParam, strHour, strParam, strMinute, strParam, strSecond, strParam, strMillonSec);

	return true;
}

#endif
