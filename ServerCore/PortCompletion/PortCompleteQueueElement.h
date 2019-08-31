
#ifndef __PORT_COMPLETE_QUEUE_ELEMENT_H__
#define __PORT_COMPLETE_QUEUE_ELEMENT_H__

#include "../../ServerLib/Include/Common/UnLockQueue.h"
#include "../../ServerLib/Include/Common/TypeDefines.h"
#include "../../Interface/ICompletionPort.h"

#include "PortCompleteQueueElementDataDefine.h"

#define MSG_BUFFER_COUNT	2048



#ifdef _WIN_

typedef struct _PER_IO_CONTEXT OPERATE_IO_CONTEXT;
typedef struct _PER_SOCKET_CONTEXT OPERATE_SOCKET_CONTEXT;

class SocketMessageData : public UnLockQueueElementDataBase
{
public:
	SocketMessageData();
	virtual ~SocketMessageData();

	SocketMessageData& operator=(const UnLockQueueElementDataBase rhv) override;
	SocketMessageData& operator=(const SocketMessageData rhv);

	uint64*					pSocket;
	char					strMesBuffer[IO_BUFFER_SIZE];
	uint32					uBufferSize;
	int64					nStoreID;
};

class SocketRegisterData : public UnLockQueueElementDataBase
{
public:
	SocketRegisterData();
	virtual ~SocketRegisterData();

	SocketRegisterData& operator=(const UnLockQueueElementDataBase rhv) override;
	SocketRegisterData& operator=(const SocketRegisterData rhv);

	PortCompletionSocketRegisterType	eRegisterType;
	OPERATE_SOCKET_CONTEXT*				pSocketContext;
	int									nThreadID;
};
#endif	//	_WIN_

#endif	//	__PORT_COMPLETE_QUEUE_ELEMENT_H__


