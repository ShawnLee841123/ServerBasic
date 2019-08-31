/*
	替代主线程的创建完成端口之后的链接监听工作
*/



#ifndef __PORT_COMPLETE_LISTENER_H__
#define __PORT_COMPLETE_LISTENER_H__

#include "PortCompleteWorker.h"
#include "../../Interface/ICompletionPort.h"

#ifdef _WIN_
#include <mswsock.h>
#endif

class PortCompleteListener : public PortCompleteWorker
{
public:
	PortCompleteListener();
	virtual ~PortCompleteListener();

	virtual bool SetCompletionPort(void* pPort) override;

#pragma region parent function override
	virtual bool OnThreadInitialize(int nTickTime) override;
	virtual bool OnThreadStart(int nThreadID) override;
	virtual bool OnThreadDestroy() override;
	virtual bool OnThreadRunning() override;
#pragma endregion
protected:
#pragma region function
#ifdef _WIN_
	virtual bool OnWindowsListenThreadStart();

	virtual bool OnWindowsListenLoop();
#endif

#pragma endregion


	uint64					m_ListenSocket;
#ifdef _WIN_
	LPFN_ACCEPTEX			m_lpFunAcceptEx;

	LPPER_SOCKET_CONTEXT		m_pHandleData;
	LPPER_IO_CONTEXT			m_pIoData;
	LPPER_IO_CONTEXT			m_pListenIoData;
#endif
};

#endif	//	__PORT_COMPLETE_LISTENER_H__
