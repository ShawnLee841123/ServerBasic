

#include "../../Include/Common/LibThreadBase.h"
#include "../../Include/Common/UnLockQueue.h"
#include "../../Include/Common/StandardUnLockElement.h"
#include "../../Include/System/TimeSystem.h"
#include <stdarg.h>

//	Linux下需要引入的库
#ifndef _WIN_
#include <string.h>
#endif


ThreadBase::ThreadBase() : m_eCurStatus(ESTST_NONE), m_nThreadID(-1), m_nLogQueueID(-1), m_nTickTime(0), m_uLastTimeStamp(0)
{
	m_dicQueueKey.clear();
	memset(m_arrQueue, 0, sizeof(UnLockQueueBase*) * THREAD_QUEUE_MAX);
}

ThreadBase::~ThreadBase()
{}

bool ThreadBase::RegisterQueue(UnLockQueueBase* pQueue, const char* strQueueName, EStandQueueType eType)
{
	if (nullptr == pQueue)
		return false;

	if (nullptr == strQueueName)
	{
		THREAD_ERROR("RegisterQueue: queue name is null");
		return false;
	}

	if (0 == strQueueName[0])
	{
		THREAD_ERROR("RegisterQueue: queue name is empty");
		return false;
	}

	//	计算队列ID，以及匹配表的值
	int nCurQueueLen = (int)m_dicQueueKey.size();
	int nCurQueueFlag = eType << 16;
	int nQueueIndex = nCurQueueFlag + nCurQueueLen;
	//	日志队列，需要单独记录日志队列ID
	if (ESQT_LOG_QUEUE == eType)
		m_nLogQueueID = nCurQueueLen;

	//	设置队列
	m_arrQueue[nCurQueueLen] = pQueue;
	//	将匹配信息插入匹配表
	m_dicQueueKey.insert(std::pair<std::string, int>(strQueueName, nQueueIndex));
	return true;
}

bool ThreadBase::OnThreadInitialize(int nTickTime)
{
	m_eCurStatus = ESTST_INITIALIZED;
	m_nTickTime = nTickTime;
	return true;
}

bool ThreadBase::OnThreadStart(int nThreadID)
{
	//m_nThreadID = nThreadID;
	THREAD_MSG("Thread[%d] On start", m_nThreadID);
	m_eCurStatus = ESTST_START;
	m_Thread = std::thread(&ThreadBase::ThreadTick, this);
	return true;
}

bool ThreadBase::OnThreadRunning()
{
	m_eCurStatus = ESTST_RUNNING;
	bool bRet = true;
	bRet &= ReadQueueProcess(0);

	return bRet;
}

bool ThreadBase::OnThreadDestroy()
{
	//	先置标记
	m_eCurStatus = ESTST_DESTROIED;
	m_Thread.join();

	//	清空队列
	if (m_dicQueueKey.size() > 0)
	{
		std::map<std::string, int>::iterator iter = m_dicQueueKey.begin();
		for (; iter != m_dicQueueKey.end(); ++iter)
		{
			int nCurIndex = ((iter->second) & 0x0000FFFF);
			//	只要不是读取队列，那就是本线程的写入队列（包括日志队列）
			if (!IsReadQueueType(iter->second))
			{
				//	非读取队列需要在本线程销毁
				m_arrQueue[nCurIndex]->Destroy();
				delete m_arrQueue[nCurIndex];
			}

			//	读取队列置为空
			m_arrQueue[nCurIndex] = nullptr;
		}

		m_dicQueueKey.clear();
	}

	return true;
}

EServerThreadStatusType ThreadBase::GetThreadStatus()
{
	return m_eCurStatus;
}

int ThreadBase::GetThreadID()
{
	return m_nThreadID;
}

void ThreadBase::SetThreadID(int nThreadID)
{
	m_nThreadID = nThreadID;
}

void ThreadBase::ThreadTick()
{
	while (m_eCurStatus >= ESTST_START && m_eCurStatus < ESTST_DESTROIED)
	{
		if (m_uLastTimeStamp == 0)
			m_uLastTimeStamp = GetTimeStamp();

		OnThreadRunning();
		uint32 uCurTimeStamp = GetTimeStamp();
		int nDelta = int(uCurTimeStamp - m_uLastTimeStamp);
		m_uLastTimeStamp = uCurTimeStamp;
		m_eCurStatus = ESTST_SLEPT;
		int nSleepTime = m_nTickTime - nDelta;
		if (nSleepTime > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(nSleepTime));
	}
}

bool ThreadBase::ReadQueueProcess(int nElapse)
{
	if (m_dicQueueKey.size() <= 0)
		return true;

	std::map<std::string, int>::iterator iter = m_dicQueueKey.begin();
	for (; iter != m_dicQueueKey.end(); ++iter)
	{
		if (!IsReadQueueType(iter->second))
			continue;

		int nQueueIndex = ((iter->second) & 0x0000FFFF);
		UnLockQueueBase* pQueue = m_arrQueue[nQueueIndex];
		if (nullptr == pQueue)
		{
			THREAD_ERROR("Func[ReadQueueProcess] Queue[%s] is null", iter->first.c_str());
			continue;
		}

		EQueueOperateResultType eRet = EQORT_SUCCESS;
		do 
		{
			UnLockQueueElementBase* pElement = pQueue->PopQueueElement(eRet);
			if (nullptr == pElement)
			{
				THREAD_MSG("Empty Thread[%d] Queue[%s]", m_nThreadID, iter->first.c_str());
				break;
			}

			if (!OnQueueElement(pElement))
			{
				eRet = EQORT_POP_INVALID_ELEMENT;
				THREAD_WARNNING("Func[ReadQueueProcess] ReadQueue[%s] have invalid element", iter->first.c_str());
				break;
			}

			pElement->ClearElement();

		} while (eRet == EQORT_SUCCESS);
	}

	return true;
}

bool ThreadBase::OnQueueElement(UnLockQueueElementBase* pElement)
{
	if (nullptr == pElement)
	{
		THREAD_ERROR("Func[OnQueueElement] Param[pElement] is null");
		return false;
	}

	return true;
}

bool ThreadBase::AddQueueElement(UnLockQueueElementBase* pElement, const char* strQueueName)
{
	if (nullptr == strQueueName)
	{
		THREAD_ERROR("Func[AddQueueElement] Param[strQueueName] is null");
		return false;
	}

	if (0 == strQueueName[0])
	{
		THREAD_ERROR("Func[AddQueueElement] Param[strQueueName] is Empty string");
		return false;
	}

	if (nullptr == pElement)
	{
		THREAD_ERROR("Func[AddQueueElement] Param[pElement] is null");
		return false;
	}
		
	std::map<std::string, int>::iterator iter = m_dicQueueKey.find(strQueueName);
	if (iter == m_dicQueueKey.end())
	{
		THREAD_ERROR("Func[AddQueueElement] Can not find write queue[%s]", strQueueName);
		return false;
	}

	if (!IsWriteQueueType(iter->second))
	{
		THREAD_ERROR("Func[AddQueueElement] Queue[%s] is not Write queue", strQueueName);
		return false;
	}

	int nQueueIndex = ((iter->second) & 0x0000FFFF);
	UnLockQueueBase* pQueue = m_arrQueue[nQueueIndex];
	if (nullptr == pQueue)
	{
		THREAD_ERROR("Func[AddQueueElement] Queue[%s] is null", strQueueName);
		return false;
	}

	EQueueOperateResultType eRet = pQueue->PushQueueElement(pElement);
	return EQORT_SUCCESS == eRet;
}

int ThreadBase::GetQueueID(const char* strQueueName)
{
	if (nullptr == strQueueName)
		return -1;

	if (strQueueName[0] == 0)
		return -1;

	std::map<std::string, int>::iterator iter = m_dicQueueKey.find(strQueueName);
	if (iter != m_dicQueueKey.end())
		return iter->second;

	return -1;
}

bool ThreadBase::IsReadQueueType(int nQueueID)
{
	return ((nQueueID >> 16) == ESQT_READ_QUEUE);
}

bool ThreadBase::IsWriteQueueType(int nQueueID)
{
	return ((nQueueID >> 16) == ESQT_WRITE_QUEUE);
}

bool ThreadBase::IsLogQueue(int nQueueID)
{
	return ((nQueueID >> 16) == ESQT_LOG_QUEUE);
}

bool ThreadBase::outputLog(const char* strLog, int nLogLevel)
{
	if (m_nLogQueueID < 0)
		return false;

	LogQueueElementData oData;
	int nLogLen = (int)strlen(strLog) + 1;
	memcpy(oData.strLog, strLog, nLogLen);
	oData.nLogLevel = nLogLevel;
	oData.nThreadID = m_nThreadID;

	EQueueOperateResultType eRet = m_arrQueue[m_nLogQueueID]->PushQueueElement(&oData, sizeof(oData));
	return EQORT_SUCCESS == eRet;
}

bool ThreadBase::Output(int nLevel, const char* strLog, ...)
{
	if (nullptr == strLog)
		return false;

	if (strcmp(strLog, "") == 0)
		return false;

	if (m_nLogQueueID < 0)
		return false;

	if (nLevel > ELLT_ERROR)
		return false;

	if (nLevel < ELLT_ECHO)
		return false;

	LogQueueElementData oData;
	va_list ap;
	va_start(ap, strLog);
	vsnprintf(oData.strLog, LOG_CHARACTER_MAX, strLog, ap);
	va_end(ap);

	oData.nLogLevel = nLevel;
	oData.nThreadID = m_nThreadID;
	EQueueOperateResultType eRet = m_arrQueue[m_nLogQueueID]->PushQueueElement(&oData, sizeof(oData));
	return EQORT_SUCCESS == eRet;
}
