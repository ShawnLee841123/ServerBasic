
#ifndef __UNLOCK_QUEUQ_ELEMENT_DEFINE_H__
#define __UNLOCK_QUEUQ_ELEMENT_DEFINE_H__

#pragma region Log data
#include "UnLockQueue.h"



//	日志打印数据
#define LOG_CHARACTER_MAX	512
class LogQueueElementData : public UnLockQueueElementDataBase
{
public:
	LogQueueElementData();
	virtual ~LogQueueElementData();

	LogQueueElementData& operator=(const UnLockQueueElementDataBase rhv) override;
	LogQueueElementData& operator=(const LogQueueElementData rhv);

	int			nLogLevel;		//	日志等级
	int			nThreadID;		//	线程ID
	char		strLog[LOG_CHARACTER_MAX];
};

class RegisterLogQueueData : public UnLockQueueElementDataBase
{
public:
	RegisterLogQueueData();
	virtual ~RegisterLogQueueData();

	RegisterLogQueueData& operator=(const UnLockQueueElementDataBase rhv) override;
	RegisterLogQueueData& operator=(const RegisterLogQueueData rhv);

	int						nThreadID;
	bool					bRegister;
	UnLockQueueBase*		pThreadLogQueue;
};

#pragma endregion

#endif	//	__UNLOCK_QUEUQ_ELEMENT_DEFINE_H__
