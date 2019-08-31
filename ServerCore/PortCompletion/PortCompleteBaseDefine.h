
#ifndef __PORT_COMPLETE_BASE_DEFINE_H__

#define __PORT_COMPLETE_BASE_DEFINE_H__

#include "../../ServerLib/Include/Common/TypeDefines.h"
#include <vector>
#include "PortCompleteQueueElementDataDefine.h"

#ifdef _WIN_
//#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#define SAFE_RELEASE_SOCKET(a) {if (INVALID_SOCKET != a){closesocket(a); a = INVALID_SOCKET;}}
//	单IO操作数据		用于sockect的每一个操作
typedef struct _PER_IO_CONTEXT
{
	WSAOVERLAPPED overlap;							//	每个重叠的操作结构，针对socket的每个操作，都要有一个	在结构中必须是第一个
	WSABUF buffer;								//	重叠操作参数缓冲区
	char databuf[IO_BUFFER_SIZE];				//	字符缓冲区
	int datalength;								//	字符串长度
	ECompletionPortOperateType operateType;		//	操作类型
	SOCKET link;								//	此操作使用的socket

	_PER_IO_CONTEXT();
	virtual ~_PER_IO_CONTEXT();
	_PER_IO_CONTEXT& operator=(const _PER_IO_CONTEXT rhv);
	void ResetDataBuff();
	void ResetOverlapBuff();
	void ResetBuff();
}OPERATE_IO_CONTEXT, *LPOPERATE_IO_CONTEXT;


//	单句柄定义		//	用于单个socket
typedef struct _PER_SOCKET_CONTEXT
{
	SOCKET								link;				//	客户端连接
	SOCKADDR_IN							clientAddr;			//	客户端地址
	int									RecvThreadID;		//	接收消息线程ID
	int									SendThreadID;		//	发送消息线程ID
	std::vector<LPOPERATE_IO_CONTEXT>		vIoContext;			//	IO操作队列
	int64								storeID;

	_PER_SOCKET_CONTEXT();
	virtual ~_PER_SOCKET_CONTEXT();
	_PER_SOCKET_CONTEXT& operator=(const _PER_SOCKET_CONTEXT rhv);
	void ClearOperate();
	LPOPERATE_IO_CONTEXT GetNewIoOperate();
	void RemoveIoOperate(LPOPERATE_IO_CONTEXT pOperate);

}OPERATE_SOCKET_CONTEXT, *LPOPERATE_SOCKET_CONTEXT;

#endif	//	_WIN_

#endif	//	__PORT_COMPLETE_BASE_DEFINE_H__

