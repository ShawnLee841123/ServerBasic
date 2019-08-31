

#ifndef __LOG_THREAD_BASE_H__
#define __LOG_THREAD_BASE_H__

#include "LibThreadBase.h"

#define LOG_FLAG_CHARACTER_MAX 32
#define LOG_TIME_CHAR_SIZE 32
#define THREAD_LOG_NAME_CHARACTER 64
#define THREAD_LOG_MAX 512

class LogQueueElementData;
class RegisterLogQueueData;

class LogThreadBase : public ThreadBase
{
public:
	LogThreadBase();
	virtual ~LogThreadBase();
#pragma region Thread function override
	virtual bool OnThreadDestroy();
#pragma endregion
	//	在log线程执行之前执行,windows下需要给出console句柄（屏幕输出打印更换颜色使用）
	virtual bool BeforeLogStart(int nScreenLevel, int nFileLevel, void* pLogFile, void* pConsole = nullptr);
protected:

#pragma region Thread function override
	virtual bool OnQueueElement(UnLockQueueElementBase* pElement);
	//	因为log线程的队列有点特殊，全部都是读取队列，没有log队列，也没有写入队列，这里就要继承出来搞点事情
	virtual bool ReadQueueProcess(int nElapse);	
#pragma endregion

#pragma region Queue element Process
	virtual bool OnLogoutElement(LogQueueElementData* pData);
	virtual bool OnRegisterLogElement(RegisterLogQueueData* pData);
#pragma endregion

#pragma region Log string about
	virtual bool OutputStringToScreen(const char* strValue, int nLevel);
	virtual bool OutputStringToFile(const char* strValue, int nLevel);
	virtual bool GetLogoutString(const char* strValue, char* strOut, int nThreadID, int nLevel);
#pragma endregion

	ELogLevelType							m_ScreenOutputLevel;		//	屏幕打印等级
	ELogLevelType							m_FileOutputLevel;			//	日志打印等级
	void*									m_pLogFile;					//	日志文件句柄
	void*									m_pConsole;					//	控制台句柄（windows下使用；liunx没有查到，貌似木有）
};

extern char g_LogFlag[][LOG_FLAG_CHARACTER_MAX];

#endif	//__LOG_THREAD_BASE_H__

