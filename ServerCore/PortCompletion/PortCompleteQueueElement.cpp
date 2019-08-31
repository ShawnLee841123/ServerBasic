
#include "PortCompleteQueueElement.h"
#include "PortCompleteBaseDefine.h"

SocketMessageData::SocketMessageData(): pSocket(nullptr), uBufferSize(0), nStoreID(0)
{
	DataID = EESDGT_NONE;
	memset(strMesBuffer, 0, sizeof(char) * IO_BUFFER_SIZE);
}

SocketMessageData::~SocketMessageData()
{}

SocketMessageData& SocketMessageData::operator=(const UnLockQueueElementDataBase rhv)
{
	DataID = rhv.GetDataID();
	return *this;
}

SocketMessageData& SocketMessageData::operator=(const SocketMessageData rhv)
{
	DataID = rhv.DataID;
	pSocket = rhv.pSocket;
	uBufferSize = rhv.uBufferSize;
	memcpy(strMesBuffer, rhv.strMesBuffer, IO_BUFFER_SIZE);
	nStoreID = rhv.nStoreID;
	
	return *this;
}

SocketRegisterData::SocketRegisterData():pSocketContext(nullptr), nThreadID(0), eRegisterType()
{
	DataID = EESDGT_REGISTER;
}
SocketRegisterData::~SocketRegisterData()
{
	//	其他都好办，pSocketContext绝对不允许释放。这里释放就要拉清丹
}

SocketRegisterData& SocketRegisterData::operator=(const UnLockQueueElementDataBase rhv)
{
	DataID = rhv.GetDataID();
	return *this;
}

SocketRegisterData& SocketRegisterData::operator=(const SocketRegisterData rhv)
{
	DataID = rhv.DataID;
	pSocketContext = rhv.pSocketContext;
	nThreadID = rhv.nThreadID;
	eRegisterType = rhv.eRegisterType;

	return *this;
}

