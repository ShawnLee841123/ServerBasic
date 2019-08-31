
#ifndef __INTERFACE_SERVER_CORE_H__
#define __INTERFACE_SERVER_CORE_H__

class IServerCore
{
public:
	IServerCore(){}
	virtual ~IServerCore(){}

	//	初始化
	virtual bool Initialize(IServerCore* pCoreRoot) = 0;
	//	开始
	virtual bool Start() = 0;
	//	循环
	virtual bool Tick(int nElapse) = 0;
	//	销毁
	virtual bool Destroy() = 0;
	//	获取核心根
	virtual IServerCore* GetCoreRoot() = 0;
	//	注册
	virtual bool RegisterServerCore(IServerCore* pCore) = 0;
	//	获取名称
	virtual const char* GetName() = 0;

#pragma region Log About
	//	创建日志
	virtual void RegisterLogFile(const char* strLogName) = 0;
	//	输出日志相关
	virtual void LogOutput(int nLevel, const char* strValue, ...) = 0;
#pragma endregion

protected:
	//	初始化
	virtual bool OnInitialize() = 0;
	//	开始
	virtual bool OnStart() = 0;
	//	循环
	virtual bool OnTick(int nElapse) = 0;
	//	销毁
	virtual bool OnDestroy() = 0;
};

#pragma region Log About 
#define CORE_ECHO(a, ...) LogOutput(1, a, ##__VA_ARGS__)
#define CORE_DEBUG(a, ...) LogOutput(2, a, ##__VA_ARGS__)
#define CORE_MSG(a, ...) LogOutput(3, a, ##__VA_ARGS__)
#define CORE_WARNING(a, ...) LogOutput(4, a, ##__VA_ARGS__)
#define CORE_ERROR(a, ...) LogOutput(5, a, ##__VA_ARGS__)
#pragma endregion

#endif	//__INTERFACE_SERVER_CORE_H__
