

#include "PortCompleteListener.h"
#include "../../Interface/ICompletionPort.h"
//#include <winsock.h>
#include <ws2tcpip.h>

PortCompleteListener::PortCompleteListener() : m_lpFunAcceptEx(nullptr), m_ListenSocket(0xFFFFFFFFFFFFFFFF), m_pHandleData(nullptr), m_pIoData(nullptr)
{}

PortCompleteListener::~PortCompleteListener()
{}

bool PortCompleteListener::SetCompletionPort(void* pPort)
{
	return PortCompleteWorker::SetCompletionPort(pPort);
}

#pragma region parent function override
bool PortCompleteListener::OnThreadInitialize(int nTickTime) 
{
#ifdef _WIN_
#pragma region Create windows socket
	WSADATA waadata;
	int nError;
	nError = WSAStartup(MAKEWORD(2, 2), &waadata);
	//	版本号先不做检查

	sockaddr_in serverAddr;
#pragma region Create Listen socket

	/*
		完成端口的socket初始化必须使用WSAsocket初始化

		最后一个参数必须是WSA_FLAG_OVERLAPPED（后续会写详细参数解析）
	*/
	m_ListenSocket = (uint64)WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
#pragma endregion
	ZeroMemory((char*)&serverAddr, sizeof(sockaddr_in));
	//serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "10.53.3.212:11111", &serverAddr);
	//serverAddr.sin_port = htons(11111);

	if (nullptr == CreateIoCompletionPort((HANDLE)m_ListenSocket, m_pCompletionPort, (DWORD)m_pIoData, 0))
	{
		closesocket((SOCKET)m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return false;
	}

	if (SOCKET_ERROR == bind(m_ListenSocket, (sockaddr*)&serverAddr, sizeof(sockaddr)))
	{
		closesocket((SOCKET)m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return false;
	}

	listen(m_ListenSocket, LISTEN_LINK_COUNT);
	
#pragma endregion

	m_lpFunAcceptEx = nullptr;
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	if (0 == WSAIoctl(m_ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &m_lpFunAcceptEx, sizeof(m_lpFunAcceptEx), &dwBytes, NULL, NULL))
	{ }
	else
	{
		int nError = WSAGetLastError();
		return false;
	}

	return true;
#endif

	return false;
}

bool PortCompleteListener::OnThreadStart(int nThreadID)
{
	//	投递监听请求后，再启动线程
	bool bRet = true;
#ifdef _WIN_
	bRet &= OnWindowsListenThreadStart();
#else
	bRet &= false;
#endif
	bRet &= PortCompleteWorker::OnThreadStart(nThreadID);
	return bRet;
}

bool PortCompleteListener::OnThreadDestroy()
{
	bool bRet = true;
	
	return bRet;
}
bool PortCompleteListener::OnThreadRunning()
{
	bool bRet = true;
	bRet &= OnWindowsListenLoop();

	return bRet;
}
#pragma endregion

#pragma region function
#ifdef _WIN_
bool PortCompleteListener::OnWindowsListenThreadStart()
{
	//LPPER_IO_CONTEXT perIoData = new PER_IO_CONTEXT();
	if (nullptr == m_pListenIoData)
		m_pListenIoData = new LPPER_IO_CONTEXT();

	memset(&(m_pListenIoData->overlapped), 0, sizeof(OVERLAPPED));
	m_pListenIoData->operateType = ECPOT_ACCEPT;
	m_pListenIoData->link = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	m_pListenIoData->datalength = IO_BUFFER_SIZE;
	DWORD flags = 0;
	m_lpFunAcceptEx(m_ListenSocket, m_pListenIoData->link, m_pListenIoData->databuf, m_pListenIoData->datalength - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &flags, &m_pListenIoData->overlapped);

	return true;
}

bool PortCompleteListener::OnWindowsListenLoop()
{
	DWORD dwBytes = 0;
	LPPER_SOCKET_CONTEXT pHandleData = nullptr;
	LPPER_IO_CONTEXT pIoData = nullptr;
	GetQueuedCompletionStatus(m_pCompletionPort, &dwBytes, (PULONG_PTR)&pHandleData, (LPOVERLAPPED*)&m_pListenIoData, INFINITE);
	if (pIoData->operateType != ECPOT_ACCEPT)
	{
		THREAD_WARNNING("Repeat socket");
		return true;
	}

	if (SOCKET_ERROR == setsockopt(pIoData->link, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(pHandleData->socket), sizeof(pHandleData->socket)))
	{}

	CreateIoCompletionPort((HANDLE)pHandleData->socket, m_pCompletionPort, (DWORD)pHandleData, 0);
	memset(&(m_pListenIoData->overlapped), 0, sizeof(OVERLAPPED));
	m_pListenIoData->operateType = ECPOT_RECIVE;
	m_pListenIoData->buffer.buf = m_pListenIoData->databuf;
	m_pListenIoData->buffer.len = m_pListenIoData->datalength = IO_BUFFER_SIZE;

	return true;
}

#endif

#pragma endregion

