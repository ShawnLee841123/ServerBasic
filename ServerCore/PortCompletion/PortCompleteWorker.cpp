
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

#define ACCEPT_POST_MAX 10

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
	return (nullptr != m_pCompletionPort);
}

bool PortCompleteWorker::OnThreadInitialize(int nTickTime)
{
	bool bRet = true;
	bRet &= ThreadBase::OnThreadInitialize(nTickTime);
#ifdef _WIN_
	if (CheckFunctionEnable(EPCTFT_LISTEN))
	{
		bRet &= InitialListenSocket();
		bRet &= StartListenConnect();
	}
		
#endif

	return bRet;
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
	m_uThreadFunc |= (uFlag << eMask);

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
	UnLockQueueDataElementBase* pQueueEle = new UnLockQueueDataElementBase();
	pData->eRegisterType = EPCSRT_STORE;
	pData->pSocketContext = pSocketContext;
	pData->nThreadID = m_nThreadID;
	pQueueEle->SetData(pData, sizeof(SocketRegisterData));

	char strQueueName[64] = { 0 };
	sprintf(strQueueName, "CoreReadQueue[%d]", m_nThreadID);
	AddQueueElement(pQueueEle, strQueueName);
	return true;
}

bool PortCompleteWorker::OnWorkerMainLoop(int nElapse)
{
	//	注意最后的参数，这里有一个十分类似的宏INFINITY,我们需要的是INFINITE
	//BOOL bRet = GetQueuedCompletionStatus((HANDLE)m_pListenContext->link, (LPDWORD)&m_dwBytesTransfered, (PULONG_PTR)&m_pLoopSockContext, (LPOVERLAPPED*)&m_pLoopOverlapped, INFINITE);
	BOOL bRet = GetQueuedCompletionStatus(m_pCompletionPort, (LPDWORD)&m_dwBytesTransfered, (PULONG_PTR)&m_pLoopSockContext, (LPOVERLAPPED*)&m_pLoopOverlapped, INFINITE);
	if (!bRet)
	{
		//	测试程序只是些了链接，完成就断开了。这里会返回错误64，客户端断开
		int nRet = WSAGetLastError();
		if (nRet == 64)
		{
			//	客户端断开连接
			OPERATE_IO_CONTEXT* pIoContext = CONTAINING_RECORD(m_pLoopOverlapped, OPERATE_IO_CONTEXT, overlap);
			//	Tell Core to Close Socket
			//	Shut down socket recv listen
			return true;
		}
		THREAD_ERROR("[PortCompleteWorker::OnWorkerMainLoop] Get completion port status faild. Error code[%d]", nRet);
		return false;
	}

	//OPERATE_IO_CONTEXT* pIoContext = (LPOPERATE_IO_CONTEXT)m_pLoopOverlapped;
	OPERATE_IO_CONTEXT* pIoContext = CONTAINING_RECORD(m_pLoopOverlapped, OPERATE_IO_CONTEXT, overlap);
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
	//	如果错误号为10093，先确认是否初始化socket
	if (INVALID_SOCKET == m_pListenContext->link)
	{
		int nError = WSAGetLastError();
		THREAD_ERROR("Thread Create Listen Socket Faild. Error Code[%d]", nError);
		return false;
	}

	//	bind listen socket to completion port
	if (nullptr == CreateIoCompletionPort((HANDLE)m_pListenContext->link, m_pCompletionPort, (DWORD)m_pListenContext, 0))
	{
		int nError = WSAGetLastError();
		THREAD_ERROR("Bind listen socket to completion port faild. Error Code[%d]", nError);
		SAFE_RELEASE_SOCKET(m_pListenContext->link);
		return false;
	}

	//	bind listen socket to server address
	//sockaddr_in serverAddr;
	CORE_SOCKETADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(CORE_SOCKETADDR_IN));
	//	字符串转Socket地址(地址不能带有端口号，不然报错)
	inet_pton(AF_INET, "10.53.3.212", &(serverAddr.sin_addr));
	serverAddr.sin_port = htons(11111);
	//	family一定要设置，不然就10047了
	serverAddr.sin_family = AF_INET;

	if (SOCKET_ERROR == bind(m_pListenContext->link, (CORE_SOCKADDR*)&(serverAddr), sizeof(CORE_SOCKADDR)))
	{
		int nError = WSAGetLastError();
		THREAD_ERROR("Can not bind listen socket to server address. Error Code[%d]", nError);
		SAFE_RELEASE_SOCKET(m_pListenContext->link);
		return false;
	}

	if (SOCKET_ERROR == listen(m_pListenContext->link, LISTEN_LINK_COUNT))
	{
		int nError = WSAGetLastError();
		THREAD_ERROR("Thread Start Listen Error[%d]", nError);
		return false;
	}

	//	Get Windows extra function pointer AcceptEx and GetAcceptExSockAddrs 
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(m_pListenContext->link, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &m_pFnAcceptEx, sizeof(m_pFnAcceptEx), &dwBytes, NULL, NULL))
	{
		int nError = WSAGetLastError();
		THREAD_ERROR("Can not get Function[AcceptEx] pointer. Error code[%d]", nError);
		SAFE_RELEASE_SOCKET(m_pListenContext->link);
		return false;
	}

	GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(m_pListenContext->link, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockAddrs, sizeof(guidGetAcceptExSockAddrs),
		&m_pFnGetAcceptExSockAddrs, sizeof(m_pFnGetAcceptExSockAddrs), &dwBytes, NULL, NULL))
	{
		int nError = WSAGetLastError();
		THREAD_ERROR("Can not get Function[GetAcceptExSockAddrs] pointer. Error code[%d]", nError);
		SAFE_RELEASE_SOCKET(m_pListenContext->link);
		return false;
	}

	m_pLoopOverlapped = new OVERLAPPED();

	return true;
}

bool PortCompleteWorker::StartListenConnect()
{
	if (!CheckFunctionEnable(EPCTFT_LISTEN))
		return true;

	for (int i = 0; i < ACCEPT_POST_MAX; i++)
	{
		//	投递第一个accept请求
		OPERATE_IO_CONTEXT* pAcceptIoContext = m_pListenContext->GetNewIoOperate();
		if (!PostAccept(m_pListenContext, pAcceptIoContext))
		{
			m_pListenContext->RemoveIoOperate(pAcceptIoContext);
			return false;
		}
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
	//	下面这部分注释掉的原因和LPFN_ACCEPTEX的调用时一样的
	//pFn(pIoContext->buffer.buf, pIoContext->buffer.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
	//	sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&pLocalAddr, &nLocalAddrLen, (LPSOCKADDR*)&pClientAddr, &nClientAddrLen);

	pFn(pIoContext->buffer.buf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&pLocalAddr, &nLocalAddrLen, (LPSOCKADDR*)&pClientAddr, &nClientAddrLen);

#pragma region Create new socket for recv client message or Send it to Core for rearrange
	OPERATE_SOCKET_CONTEXT* pNewContext = new OPERATE_SOCKET_CONTEXT();
	//*pNewContext = *pSockContext;
	pNewContext->link = pIoContext->link;
	memcpy(&(pNewContext->clientAddr), pClientAddr, sizeof(SOCKADDR_IN));

	//	todo create queue element to core
	RegisterConnectSocket(pNewContext);
#pragma endregion
	
	m_pListenContext->RemoveIoOperate(pIoContext);
	OPERATE_IO_CONTEXT* pNewIOContext = m_pListenContext->GetNewIoOperate();
	PostAccept(m_pListenContext, pNewIOContext);

	THREAD_DEBUG("Get one Accept");
	return true;
}

bool PortCompleteWorker::DoRecv(OPERATE_SOCKET_CONTEXT* pSockContext, OPERATE_IO_CONTEXT* pIoContext)
{
	std::map<uint32, OPERATE_SOCKET_CONTEXT*>::iterator iter = m_pStoreInfo.find(pIoContext->link);
	if (iter == m_pStoreInfo.end())
	{
		THREAD_DEBUG("Can not Find store socket[%d]", pIoContext->link);
		return false;
	}

	//SOCKADDR_IN* pClientAddr = &pSockContext->clientAddr;
	CORE_SOCKETADDR_IN* pClientAddr = &(iter->second->clientAddr);
	
#pragma region Send message content to Core for logic process
	/*	
		todo:
			Create unlock queue element
			Copy message content
			Send it to core
	*/

	//	print message instead
	THREAD_DEBUG("Recive Client[%d.%d.%d.%d:%d] Message: [%s]", pClientAddr->sin_addr.S_un.S_un_b.s_b1, pClientAddr->sin_addr.S_un.S_un_b.s_b2,
		pClientAddr->sin_addr.S_un.S_un_b.s_b3, pClientAddr->sin_addr.S_un.S_un_b.s_b4, pClientAddr->sin_port, pIoContext->buffer.buf);

	//	clear this io context
	pIoContext->ResetBuff();
#pragma endregion

	//	post new recv request
	PostRecv(iter->second, pIoContext);
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

	//m_pLoopSockContext = m_pListenContext;
	LPFN_ACCEPTEX pFn = (LPFN_ACCEPTEX)m_pFnAcceptEx;
	/*
		这里有一个坑，第4个参数(pWBuff->len - ((sizeof(SOCKADDR_IN) + 16) * 2))，原意是表示pWBuff->buf用于存放数据的空间大小，如果这个值变成0，则Accept时不会等待数据，
		直接返回。之前一直都无法监听到链接，是因为一直都在等待客户端Connect之后的数据消息。只要有消息发送上来，GetQueuedCompletionStatus在Accept的时候就可以返回了。或者是将
		参数4变为0。这样在客户端Connect的时候，是可以监听到的。
	*/
	//if (FALSE == pFn(pSockContext->link, pIoContext->link, pWBuff->buf, (pWBuff->len - ((sizeof(SOCKADDR_IN) + 16) * 2)),
	//	sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, pOl))

	if (FALSE == pFn(pSockContext->link, pIoContext->link, pWBuff->buf, 0,
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

	//WorkerStoreInfo* pStoreInfo = new WorkerStoreInfo();
	//memcpy(&(pStoreInfo->Addr), &(pSockContext->clientAddr), sizeof(CORE_SOCKETADDR_IN));
	//pStoreInfo->link = pSockContext->link;
	m_pStoreInfo.insert(std::pair<uint32, OPERATE_SOCKET_CONTEXT*>(pIoContext->link, pSockContext));

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