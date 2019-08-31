
#ifndef __SERVER_CORE_BASE_H__
#define __SERVER_CORE_BASE_H__

#include "../../Interface/IServerCore.h"
#include <map>


class LogThreadBase;					//	日志线程
class UnLockQueueBase;					//	日志相关队列
class ThreadBase;

class ServerCoreBase : public IServerCore
{
public:
	ServerCoreBase();
	virtual ~ServerCoreBase();

#pragma region Parent Interface
	//	初始化
	virtual bool Initialize(IServerCore* pCoreRoot) override;

	//	开始
	virtual bool Start() override;

	//	循环
	virtual bool Tick(int nElapse) override;

	//	销毁
	virtual bool Destroy() override;

	//	获取核心根
	virtual IServerCore* GetCoreRoot() override;

	//	注册
	virtual bool RegisterServerCore(IServerCore* pCore) override;

	virtual const char* GetName() override;

#pragma region Log about
	//	创建日志
	virtual void RegisterLogFile(const char* strLogName) override;
	//	输出日志相关
	virtual void LogOutput(int nLevel, const char* strValue, ...) override;
#pragma endregion
#pragma endregion

	virtual bool ThreadRegisterLog(ThreadBase* pThread, const char* strQueueName);

protected:

#pragma region Parent Interface
	virtual bool OnInitialize() override;
	virtual bool OnStart() override;
	virtual bool OnTick(int nElapse) override;
	virtual bool OnDestroy() override;
#pragma endregion 
	
#pragma region variable
	IServerCore*							m_pParentCore;				//	上级节点
	char									m_strName[64];				//	节点名称
	std::map<std::string, IServerCore*>		m_dicCoreElement;			//	子节点

#pragma region Log about
	UnLockQueueBase*						m_pLogRegisterQueue;
	UnLockQueueBase*						m_pSelfLogQueue;
	LogThreadBase*							m_pLogThread;
#pragma endregion

#pragma endregion
	
	
};

#endif
