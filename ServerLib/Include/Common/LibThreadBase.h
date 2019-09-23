
#ifndef __LIB_THREAD_BASE_H__
#define __LIB_THREAD_BASE_H__

#include "TypeDefines.h"
#include <thread>
#include <map>

class UnLockQueueBase;
class UnLockQueueElementBase;

//	线程无锁队列总数量
#define THREAD_QUEUE_MAX 64
class ThreadBase
{
public:
	ThreadBase();
	virtual ~ThreadBase();

	virtual	bool RegisterQueue(UnLockQueueBase* pQueue, const char* strQueueName, EStandQueueType eType);

	virtual bool OnThreadInitialize(int nTickTime);
	virtual bool OnThreadStart(int nThreadID);
	virtual bool OnThreadRunning();
	virtual bool OnThreadDestroy();

	EServerThreadStatusType GetThreadStatus();
	virtual int GetThreadID();
	virtual void SetThreadID(int nThreadID);

protected:
	virtual void ThreadTick();
	virtual bool ReadQueueProcess(int nElapse);
	virtual bool OnQueueElement(UnLockQueueElementBase* pElement);
	virtual bool AddQueueElement(UnLockQueueElementBase* pElement, const char* strQueueName);

	//	获取队列ID
	virtual int GetQueueID(const char* strQueueName);
	virtual bool IsReadQueueType(int nQueueID);
	virtual bool IsWriteQueueType(int nQueueID);
	virtual bool IsLogQueue(int nQueueID);
	virtual bool outputLog(const char* strLog, int nLogLevel);
	virtual bool Output(int nLevel, const char* strLog, ...);

#pragma region variable

	EServerThreadStatusType							m_eCurStatus;
	std::thread										m_Thread;				//	线程实例
	int												m_nThreadID;			//	线程ID
	int												m_nLogQueueID;			//	日志队列ID
	int												m_nTickTime;			//	每帧时间
	uint32											m_uLastTimeStamp;		//	上帧时间戳
	//队列ID = EStandQueueType << 16 + 当前匹配表数量
	std::map<std::string, int>						m_dicQueueKey;			//	队列名称和ID匹配表			
	UnLockQueueBase*								m_arrQueue[THREAD_QUEUE_MAX];				//	队列列表
	
#pragma endregion
};

#define THREAD_ECHO(a, ...) Output(1, a, ##__VA_ARGS__)
#define THREAD_DEBUG(a, ...) Output(2, a, ##__VA_ARGS__)
#define THREAD_MSG(a, ...) Output(3, a, ##__VA_ARGS__)
#define THREAD_WARNNING(a, ...) Output(4, a, ##__VA_ARGS__)
#define THREAD_ERROR(a, ...) Output(5, a, ##__VA_ARGS__)

#endif	//	__LIB_THREAD_BASE_H__

