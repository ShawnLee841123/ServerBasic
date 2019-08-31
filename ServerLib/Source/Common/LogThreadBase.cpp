#include "../../Include/Common/UnLockQueue.h"
#include "../../Include/Common/LogThreadBase.h"
#include "../../Include/Common/StandardUnLockElement.h"
#include "../../Include/Common/UnLockElementTypeDefine.h"
#include "../../Include/System/TimeSystem.h"
#include "../../Include/System/FileSystem.h"


char g_LogFlag[][LOG_FLAG_CHARACTER_MAX] =
{
	"",
	"ECHO",
	"DEBUG",
	"MSG",
	"WARNING",
	"ERROR"
};

LogThreadBase::LogThreadBase(): m_ScreenOutputLevel(ELLT_ERROR), m_FileOutputLevel(ELLT_ERROR), m_pLogFile(nullptr), m_pConsole(nullptr)
{}
LogThreadBase::~LogThreadBase(){}

//	在log线程执行之前执行,windows下需要给出console句柄（屏幕输出打印更换颜色使用）
bool LogThreadBase::BeforeLogStart(int nScreenLevel, int nFileLevel, void* pLogFile, void* pConsole)
{
	m_ScreenOutputLevel = (ELogLevelType)nScreenLevel;
	m_FileOutputLevel = (ELogLevelType)nFileLevel;
	m_pLogFile = pLogFile;
	m_pConsole = pConsole;
	return true;
}

#pragma region Thread function override
bool LogThreadBase::OnThreadDestroy()
{
	ThreadBase::OnThreadDestroy();
	if (nullptr != m_pLogFile)
	{
		fflush((FILE*)m_pLogFile);
		fclose((FILE*)m_pLogFile);
		delete m_pLogFile;
	}

	m_pLogFile = nullptr;
	m_pConsole = nullptr;

	return true;
}

bool LogThreadBase::OnQueueElement(UnLockQueueElementBase* pElement)
{
	if (nullptr == pElement)
		return false;

	UnLockQueueDataElementBase* pDataEle = dynamic_cast<UnLockQueueDataElementBase*>(pElement);
	if (nullptr == pDataEle)
		return false;

	bool bRet = false;
#pragma region need recheck desgin(disable right now)
#pragma region Log out element
	//LogQueueElementData* pLogData = dynamic_cast<LogQueueElementData*>(pData);
	//if (nullptr != pLogData)
	//{
	//	bRet = OnLogoutElement(pLogData);
	//	pElement->ClearElement();
	//	return bRet;
	//}
#pragma endregion

#pragma region Register log queue element
	//RegisterLogQueueData* pRegData = dynamic_cast<RegisterLogQueueData*>(pData);
	//if (nullptr != pRegData)
	//{
	//	bRet = OnRegisterLogElement(pRegData);
	//	pElement->ClearElement();
	//	return bRet;
	//}
#pragma endregion
#pragma endregion disable right now
	return bRet;
}

bool LogThreadBase::ReadQueueProcess(int nElapse)
{
	if (m_dicQueueKey.size() <= 0)
		return true;

	//	队列读取，只针对目前已有的队列进行操作，如果有注册或者新增的队列，那么，等下一帧再来
	int nCurQueueCount = m_dicQueueKey.size();

	//	Get register queue index. 
	//	this queue data read implement will be optimization declear soon
	int nRegisterQueueIndex = GetQueueID("RegisterQueue");
	for (int i = 0; i < nCurQueueCount; i++)
	{
		UnLockQueueBase* pQueue = m_arrQueue[i];
		if (nullptr == pQueue)
			continue;

		EQueueOperateResultType eRet = EQORT_SUCCESS;
		do
		{
			UnLockQueueDataElementBase* pElement = (UnLockQueueDataElementBase*)pQueue->PopQueueElement(eRet);
			if (nullptr == pElement)
			{
				break;
			}

			if (pElement->GetDataID() == EELDGT_PRINT)
			{
				LogQueueElementData* pData = (LogQueueElementData*)pElement->GetData();
				if (!OnLogoutElement(pData))
				{
					eRet = EQORT_POP_INVALID_ELEMENT;
					break;
				}
			}
			else if (EELDGT_REGISTER == pElement->GetDataID())
			{
				RegisterLogQueueData* pData = (RegisterLogQueueData*)pElement->GetData();
				if (!OnRegisterLogElement(pData))
				{
					eRet = EQORT_POP_INVALID_ELEMENT;
					break;
				}
			}

			//if (nRegisterQueueIndex == i)
			//{
			//	RegisterLogQueueData* pData = (RegisterLogQueueData*)pElement->GetData();
			//	if (nullptr == pData)
			//	{
			//		eRet = EQORT_POP_INVALID_ELEMENT;
			//		break;
			//	}
			//	if (!OnRegisterLogElement(pData))
			//	{
			//		eRet = EQORT_POP_INVALID_ELEMENT;
			//		break;
			//	}
			//}
			//else
			//{
			//	LogQueueElementData* pData = (LogQueueElementData*)pElement->GetData();
			//	if (nullptr == pData)
			//	{
			//		eRet = EQORT_POP_INVALID_ELEMENT;
			//		break;
			//	}

			//	if (!OnLogoutElement(pData))
			//	{
			//		eRet = EQORT_POP_INVALID_ELEMENT;
			//		break;
			//	}
			//}

			pElement->ClearElement();

		} while (eRet == EQORT_SUCCESS);
	}

	return true;
}
#pragma endregion

#pragma region Queue element Process
bool LogThreadBase::OnLogoutElement(LogQueueElementData* pData)
{
	if (nullptr == pData)
		return false;

	char strLog[THREAD_LOG_MAX] = { 0 };
	if (!GetLogoutString(pData->strLog, strLog, pData->nThreadID, pData->nLogLevel))
		return false;
	bool bRet = true;
	bRet &= OutputStringToScreen(strLog, pData->nLogLevel);
	bRet &= OutputStringToFile(strLog, pData->nLogLevel);

	return bRet;
}

bool LogThreadBase::OnRegisterLogElement(RegisterLogQueueData* pData)
{
	if (nullptr == pData)
		return false;

	if (pData->bRegister)
	{
		char strQueueName[THREAD_LOG_NAME_CHARACTER] = { 0 };
		sprintf(strQueueName, "Thread%d", pData->nThreadID);
		return RegisterQueue(pData->pThreadLogQueue, strQueueName, ESQT_READ_QUEUE);
	}

	return false;
}
#pragma endregion

#pragma region Log string about
bool LogThreadBase::OutputStringToScreen(const char* strValue, int nLevel)
{
	if (nLevel < m_ScreenOutputLevel)
		return false;

	if (nLevel < m_FileOutputLevel)
		return false;

	if (nullptr == strValue)
		return false;

	if (0 == strValue[0])
		return false;

	PrintLogTextToScreen(strValue, m_pConsole, (ELogLevelType)nLevel);
	return true;
}
bool LogThreadBase::OutputStringToFile(const char* strValue, int nLevel)
{
	if (nLevel < m_FileOutputLevel)
		return false;

	if (nullptr == strValue)
		return false;

	if (0 == strValue[0])
		return false;

	if (nullptr == m_pLogFile)
	{
		return false;
	}

	int nLen = strlen(strValue) + 1;
	fwrite(strValue, nLen, 1, (FILE*)m_pLogFile);

	return true;
}

bool LogThreadBase::GetLogoutString(const char* strValue, char* strOut, int nThreadID, int nLevel)
{
	if (nullptr == strValue)
		return false;

	if (strValue[0] == 0)
		return false;

	if (nullptr == strOut)
		return false;

	char strTime[LOG_TIME_CHAR_SIZE] = { 0 };

	if (!GetCurTimeString(strTime, LOG_TIME_CHAR_SIZE, ":"))
		return false;

	if (nullptr == strTime)
		return false;

	if (0 == strTime[0])
		return false;

	if (nLevel < ELLT_WARNING)
		sprintf(strOut, "[%s][%s]:Thread[%d]%s\n", strTime, g_LogFlag[nLevel], nThreadID, strValue);
	else if (nLevel == ELLT_WARNING)
		sprintf(strOut, "[%s][%s]:Thread[%d]%s[%s]\n", strTime, g_LogFlag[nLevel], nThreadID, strValue, g_LogFlag[nLevel]);
	else if (nLevel == ELLT_ERROR)
		sprintf(strOut, "[%s]!!!!!!!!![%s]:Thread[%d]%s[%s]!!!!!!!!!\n", strTime, g_LogFlag[nLevel], nThreadID, strValue, g_LogFlag[nLevel]);

	return true;
}
#pragma endregion