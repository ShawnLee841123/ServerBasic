
#ifndef __PORT_COMPLETE_WORKER_H__
#define __PORT_COMPLETE_WORKER_H__

#include "../../ServerLib/Include/Common/LibThreadBase.h"
#include "PortCompleteQueueElementDataDefine.h"

#ifdef _WIN_
typedef struct _PER_IO_CONTEXT OPERATE_IO_CONTEXT;
typedef struct _PER_SOCKET_CONTEXT OPERATE_SOCKET_CONTEXT;
struct WSAOVERLAPPED;
typedef struct _OVERLAPPED OVERLAPPED;
class SocketRegisterData;
#endif



class PortCompleteWorker : public ThreadBase
{
public:
	PortCompleteWorker();
	~PortCompleteWorker();

	virtual bool SetCompletionPort(void* pPort);
	virtual bool OnThreadInitialize(int nTickTime) override;
	virtual bool OnThreadRunning() override;
	virtual bool OnThreadDestroy() override;
	virtual bool WorkFunctionEnable(PortCompletionThreadFunctionMask eMask, bool bEnable);
protected:

	virtual bool CheckFunctionEnable(PortCompletionThreadFunctionMask eMask);
#ifdef _WIN_
	virtual bool RegisterConnectSocket(OPERATE_SOCKET_CONTEXT* pSocketContext);
#pragma region Loop about
	bool OnWorkerMainLoop(int nElapse);
	
	bool StartListenConnect();
	bool InitialListenSocket();

	bool DoAccept(OPERATE_SOCKET_CONTEXT* pSockContext, OPERATE_IO_CONTEXT* pIoContext);
	bool DoRecv(OPERATE_SOCKET_CONTEXT* pSockContext, OPERATE_IO_CONTEXT* pIoContext);
	bool PostAccept(OPERATE_SOCKET_CONTEXT* pSockContext, OPERATE_IO_CONTEXT* pIoContext);
	bool PostRecv(OPERATE_SOCKET_CONTEXT* pSockContext, OPERATE_IO_CONTEXT* pIoContext);

	virtual bool OnQueueElement(UnLockQueueElementBase* pElement) override;
	virtual bool OnSocketRegisterData(SocketRegisterData* pData);
#pragma endregion
#endif
#pragma region variable
	void*								m_pCompletionPort;
	uint32								m_uThreadFunc;					//	线程功能标记
#ifdef _WIN_
	void*								m_pFnAcceptEx;					//	函数指针
	void*								m_pFnGetAcceptExSockAddrs;		//	函数指针
	OPERATE_SOCKET_CONTEXT*				m_pListenContext;				//	监听结构
	uint32								m_dwBytesTransfered;

	OPERATE_SOCKET_CONTEXT*				m_pLoopSockContext;
	OVERLAPPED*							m_pLoopOverlapped;
#endif
#pragma endregion

};

#endif
