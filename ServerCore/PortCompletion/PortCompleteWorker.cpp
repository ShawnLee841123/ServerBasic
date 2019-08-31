
#include "PortCompleteWorker.h"
#include "PortCompleteBaseDefine.h"
#include "PortCompleteQueueElement.h"
#include "../../ServerLib/Include/Common/UnLockQueue.h"
#ifdef _WIN_
#include <MSWSock.h>
#pragma warning(disable:4996)
#endif

#ifdef _WIN_
PortCompleteWorker::PortCompleteWorker():m_pCompletionPort(nullptr), m_pFnAcceptEx(nullptr), m_pFnGetAcceptExSockAddrs(nullptr)
, m_pListenContext(nullptr), m_uThreadFunc(0), m_dwBytesTransfered(0), m_pLoopSockContext(nullptr)
{}
#else
PortCompleteWorker::PortCompleteWorker() : m_pCompletionPort(nullptr), m_uThreadFunc(0)
{}
#endif


PortCompleteWorker::~PortCompleteWorker()
{
	m_pCompletionPort = nullptr;
#ifdef _WIN_
	m_pFnAcceptEx = nullptr;
	m_pFnGetAcceptExSockAddrs = nullptr;
#endif
}

bool PortCompleteWorker::SetCompletionPort(void* pPort)
{
	if (nullptr == pPort)
		return false;

	if (nullptr != m_pCompletionPort)
		return false;

	m_pCompletionPort = pPort;
	return (nullptr == m_pCompletionPort);
}

bool PortCompleteWorker::OnThreadRunning()
{
	bool bRet = true;
	
	bRet &= ThreadBase::OnThreadRunning();
#ifdef _WIN_
	OnWorkerMainLoop(0);
#endif

	return bRet;
}

bool PortCompleteWorker::OnThreadDestroy()
{
	bool bRet = true;
	m_pCompletionPort = nullptr;

	bRet &= ThreadBase::OnThreadDestroy();

	return bRet;
}

bool PortCompleteWorker::WorkFunctionEnable(PortCompletionThreadFunctionMask eMask, bool bEnable)
{
	uint32 uFlag = bEnable ? 1 : 0;
	m_uThreadFunc &= (uFlag << eMask);

	return true;
}

bool PortCompleteWorker::CheckFunctionEnable(PortCompletionThreadFunctionMask eMask)
{
	uint32 uFlag = 1;
	uFlag &= (m_uThreadFunc >> eMask);

	return uFlag > 0;
}

#pragma region Loop about

#ifdef _WIN_
bool PortCompleteWorker::RegisterConnectSocket(OPERATE_SOCKET_CONTEXT* pSocketContext)
{
	SocketRegisterData* pData = new SocketRegisterData();
	UnLockQueueDataElementBase oQueueEle;
	pData->eRegisterType = EPCSRT_STORE;
	pData->pSocketContext = pSocketContext;
	pData->nThreadID = m_nThreadID;
	oQueueEle.SetData(pData, sizeof(SocketRegisterData));

	char strQueueName[64] = { 0 };
	sprintf(strQueueName, "CoreReadQueue[%d]", m_nThreadID);
	AddQueueElement(&oQueueEle, strQueueName);
	return true;
}

bool PortCompleteWorker::OnWorkerMainLoop(int nElapse)
{
	BOOL bRet = GetQueuedCompletionStatus(m_pCompletionPort, (LPDWORD)&m_dwBytesTransfered, (PULONG_PTR)&m_pLoopSockContext, (LPOVERLAPPED*)&m_pLoopOverlapped, INFINITY);
	if (!bRet)
	{
		THREAD_ERROR("[PortCompleteWorker::OnWorkerMainLoop] Get completion port status faild. Error code[%d]", WSAGetLastError());
		return false;
	}

	OPERATE_IO_CONTEXT* pIoContext = (LPOPERATE_IO_CONTEXT)m_pLoopOverlapped;
	if ((0 == m_dwBytesTransfered) && ((pIoContext->operateType == ECPOT_RECIVE)||(pIoContext->operateType == ECPOT_SEND)))
	{
		//	断开连接
		return true;
	}
	else
	{
		switch (pIoContext->operateType)
		{
		case ECPOT_ACCEPT:
			{
				if (CheckFunctionEnable(EPCTFT_LISTEN))
				{
					return DoAccept(m_pLoopSockContext, pIoContext);
				}
			}
			break;
		case ECPOT_RECIVE:
			{
				if (CheckFunctionEnable(EPCTFT_RECV))
				{
					return DoRecv(m_pLoopSockContext, pIoContext);
				}
			}
			break;
		case ECPOT_SEND:
			{
				if (CheckFunctionEnable(EPCTFT_SEND))
				{
					return true;
				}
			}
			break;
		}
	}

	return false;
}

bool PortCompleteWorker::InitialListenSocket()
{
	if (!CheckFunctionEnable(EPCTFT_LISTEN))
		return true;

	//	Create listen socket
	m_pListenContext = new OPERATE_SOCKET_CONTEXT();
	m_pListenContext->link = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_pListenContext->link)
	{
		THREAD_ERROR("Thread Create Listen Socket Faild. Error Code[%d]", WSAGetLastError());
		return false;
	}

	//	bind listen socket to completion port
	if (nullptr == CreateIoCompletionPort((HANDLE)m_pListenContext->link, m_pCompletionPort, (DWORD)m_pListenContext, 0))
	{
		THREAD_ERROR("Bind listen socket to completion port faild. Error Code[%d]", WSAGetLastError());
		SAFE_RELEASE_SOCKET(m_pListenContext->link);
		return false;
	}

	//	bind listen socket to server address
	sockaddr_in serverAddr;
	ZeroMemory((char*)&serverAddr, sizeof(sockaddr_in));
	inet_pton(AF_INET, "10.53.3.212:11111", &serverAddr);

	if (SOCKET_ERROR == bind(m_pListenContext->link, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)))
	{
		THREAD_ERROR("Can not bind listen socket to server address.");
		SAFE_RELEASE_SOCKET(m_pListenContext->link);
		return false;
	}

	//	Get Windows extra function pointer AcceptEx and GetAcceptExSockAddrs 
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(m_pListenContext->link, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &m_pFnAcceptEx, sizeof(m_pFnAcceptEx), &dwBytes, NULL, NULL))
	{
		THREAD_ERROR("Can not get Function[AcceptEx] pointer. Error code[%d]", WSAGetLastError());
		SAFE_RELEASE_SOCKET(m_pListenContext->link);
		return false;
	}

	GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(m_pListenContext->link, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockAddrs, sizeof(guidGetAcceptExSockAddrs),
		&m_pFnGetAcceptExSockAddrs, sizeof(m_pFnGetAcceptExSockAddrs), &dwBytes, NULL, NULL))
	{
		THREAD_ERROR("Can not get Function[GetAcceptExSockAddrs] pointer. Error code[%d]", WSAGetLastError());
		SAFE_RELEASE_SOCKET(m_pListenContext->link);
		return false;
	}

	return true;
}

bool PortCompleteWorker::StartListenConnect()
{
	if (!CheckFunctionEnable(EPCTFT_LISTEN))
		return true;

	if (SOCKET_ERROR == listen(m_pListenContext->link, LISTEN_LINK_COUNT))
	{
		THREAD_ERROR("Thread Start Listen Error");
		return false;
	}

	//	投递第一个accept请求
	OPERATE_IO_CONTEXT* pAcceptIoContext = m_pListenContext->GetNewIoOperate();
	if (!PostAccept(m_pListenContext, pAcceptIoContext))
	{
		m_pListenContext->RemoveIoOperate(pAcceptIoContext);
		return false;
	}

	THREAD_DEBUG("Worker thread start listen");
	return true;
}

bool PortCompleteWorker::DoAccept(OPERATE_SOCKET_CONTEXT* pSockContext, OPERATE_IO_CONTEXT* pIoContext)
{
	//	Create socket addr object for store socket information
	SOCKADDR_IN* pClientAddr = nullptr;
	SOCKADDR_IN* pLocalAddr = nullptr;
	int nClientAddrLen = sizeof(SOCKADDR_IN), nLocalAddrLen = sizeof(SOCKADDR_IN);
	//	Get Socket information
	LPFN_GETACCEPTEXSOCKADDRS pFn = (LPFN_GETACCEPTEXSOCKADDRS)m_pFnGetAcceptExSockAddrs;
	pFn(pIoContext->buffer.buf, pIoContext->buffer.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&pLocalAddr, &nLocalAddrLen, (LPSOCKADDR*)&pClientAddr, &nClientAddrLen);

#pragma region Create new socket for recv client message or Send it to Core for rearrange
	OPERATE_SOCKET_CONTEXT* pNewContext = new OPERATE_SOCKET_CONTEXT();
	*pNewContext = *pSockContext;

	//	todo create queue element to core
	RegisterConnectSocket(pSockContext);
#pragma endregion
	
	pIoContext->ResetDataBuff();
	PostAccept(pSockContext, pIoContext);

	return true;
}

bool PortCompleteWorker::DoRecv(OPERATE_SOCKET_CONTEXT* pSockContext, OPERATE_IO_CONTEXT* pIoContext)
{
	SOCKADDR_IN* pClientAddr = &pSockContext->clientAddr;
	
#pragma region Send message content to Core for logic process
	/*	
		todo:
			create unlock queue element 
			send it to core
	*/

	//	print message instead
	THREAD_DEBUG("Recive Client[%d.%d.%d.%d:%d] Message: [%s]", pClientAddr->sin_addr.S_un.S_un_b.s_b1, pClientAddr->sin_addr.S_un.S_un_b.s_b2,
		pClientAddr->sin_addr.S_un.S_un_b.s_b3, pClientAddr->sin_addr.S_un.S_un_b.s_b4, pClientAddr->sin_port, pIoContext->buffer.buf);

	//	clear this io context
	pIoContext->ResetBuff();
#pragma endregion

	//	post new recv request
	PostRecv(pSockContext, (OPERATE_IO_CONTEXT*)pSockContext->GetNewIoOperate());
	return true;
}

bool PortCompleteWorker::PostAccept(OPERATE_SOCKET_CONTEXT* pSockContext, OPERATE_IO_CONTEXT* pIoContext)
{
	if (INVALID_SOCKET == pSockContext->link)
	{
		THREAD_ERROR("Listen socket INVALID");
		return false;
	}

	DWORD dwBytes = 0;
	pIoContext->operateType = ECPOT_ACCEPT;
	WSABUF* pWBuff = &pIoContext->buffer;
	WSAOVERLAPPED* pOl = &pIoContext->overlap;
	pIoContext->link = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == pIoContext->link)
	{
		THREAD_ERROR("Create new socket for client faild");
		return false;
	}

	LPFN_ACCEPTEX pFn = (LPFN_ACCEPTEX)m_pFnAcceptEx;
	if (FALSE == pFn(pSockContext->link, pIoContext->link, pWBuff->buf, (pWBuff->len - ((sizeof(SOCKADDR_IN) + 16) * 2)),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, pOl))
	{
		int nErrorCode = WSAGetLastError();
		if (WSA_IO_PENDING != nErrorCode)
		{
			THREAD_ERROR("Post AcceptEx request faild. Error code[%d]", nErrorCode);
			return false;
		}
	}

	return true;
}

bool PortCompleteWorker::PostRecv(OPERATE_SOCKET_CONTEXT* pSockContext, OPERATE_IO_CONTEXT* pIoContext)
{
	if (nullptr == pIoContext)
		pIoContext = pSockContext->GetNewIoOperate();

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF* pWbuf = &pIoContext->buffer;
	WSAOVERLAPPED* pOl = &pIoContext->overlap;
	pIoContext->ResetBuff();
	pIoContext->operateType = ECPOT_RECIVE;

	int nByteRecv = WSARecv(pIoContext->link, pWbuf, 1, &dwBytes, &dwFlags, pOl, nullptr);
	int nErrorCode = WSAGetLastError();
	if ((SOCKET_ERROR == nByteRecv) && (WSA_IO_PENDING != nErrorCode))
	{
		THREAD_ERROR("Post recv request faild");
		return false;
	}

	return true;
}

bool PortCompleteWorker::OnQueueElement(UnLockQueueElementBase* pElement)
{
	UnLockQueueDataElementBase* pDataElement = dynamic_cast<UnLockQueueDataElementBase*>(pElement);
	if (nullptr == pDataElement)
		return true;

	uint32 uDataID = pDataElement->GetDataID();
	switch (uDataID)
	{
		case EESDGT_REGISTER:
			{
				SocketRegisterData* pData = (SocketRegisterData*)pDataElement->GetData();
				return OnSocketRegisterData(pData);
			}
			break;
	}

	return true;
}

bool PortCompleteWorker::OnSocketRegisterData(SocketRegisterData* pData)
{
	if (!CheckFunctionEnable(EPCTFT_RECV))
	{
		THREAD_ERROR("[PortCompleteWorker::OnSocketRegisterData] Recv function should not be have on Thread[%d]", m_nThreadID);
		return true;
	}

	if (pData->eRegisterType != EPCSRT_RECV)
	{
		THREAD_ERROR("[PortCompleteWorker::OnSocketRegisterData] Register socket operater should not be type[%d]", pData->eRegisterType);
		return true;
	}

	OPERATE_SOCKET_CONTEXT* pSockContext = pData->pSocketContext;
	LPOPERATE_IO_CONTEXT pIoContext = pSockContext->GetNewIoOperate();
	pIoContext->operateType = ECPOT_RECIVE;
	pIoContext->link = pSockContext->link;

	pSockContext->RecvThreadID = m_nThreadID;
	if (nullptr == CreateIoCompletionPort((HANDLE)pSockContext->link, m_pCompletionPort, (DWORD)&pSockContext, 0))
	{
		THREAD_ERROR("[PortCompleteWorker::OnSocketRegisterData] Bind socket[%d] completion port faild", pSockContext->storeID);
		return true;
	}

	PostRecv(pSockContext, (OPERATE_IO_CONTEXT*)pIoContext);
	return true;
}
#endif
#pragma endregion