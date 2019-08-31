
#include "../../Include/Common/StandardUnLockElement.h"
#include "../../Include/Common/UnLockElementTypeDefine.h"
#include <string>

#pragma region Log data

LogQueueElementData::LogQueueElementData() : nThreadID(-1), nLogLevel(0)
{
	DataID = EELDGT_PRINT;
	memset(strLog, 0, sizeof(char) * LOG_CHARACTER_MAX);
}

LogQueueElementData::~LogQueueElementData()
{
}

LogQueueElementData& LogQueueElementData::operator=(const UnLockQueueElementDataBase rhv)
{
	DataID = rhv.GetDataID();
	return *this;
}

LogQueueElementData& LogQueueElementData::operator=(const LogQueueElementData rhv)
{
	DataID = rhv.DataID;
	nThreadID = rhv.nThreadID;
	nLogLevel = rhv.nLogLevel;
	memcpy(strLog, rhv.strLog, LOG_CHARACTER_MAX);
	return *this;
}

RegisterLogQueueData::RegisterLogQueueData() : nThreadID(-1), bRegister(false), pThreadLogQueue(nullptr)
{
	DataID = EELDGT_REGISTER;
}

RegisterLogQueueData::~RegisterLogQueueData()
{}

RegisterLogQueueData& RegisterLogQueueData::operator=(const UnLockQueueElementDataBase rhv)
{
	DataID = rhv.GetDataID();
	return *this;
}

RegisterLogQueueData& RegisterLogQueueData::operator=(const RegisterLogQueueData rhv)
{
	DataID = rhv.DataID;
	nThreadID = rhv.nThreadID;
	bRegister = rhv.bRegister;
	pThreadLogQueue = rhv.pThreadLogQueue;
	return *this;
}

#pragma endregion

