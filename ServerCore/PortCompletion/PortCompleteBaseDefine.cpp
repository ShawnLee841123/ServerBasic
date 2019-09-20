#include "PortCompleteBaseDefine.h"
#ifdef _WIN_

#pragma region IO context

_PER_IO_CONTEXT::_PER_IO_CONTEXT() : datalength(0), link(INVALID_SOCKET), operateType(ECPOT_NONE)
{
	ResetBuff();
}

_PER_IO_CONTEXT::~_PER_IO_CONTEXT()
{
	ResetBuff();
	//SAFE_RELEASE_SOCKET(link);
}

_PER_IO_CONTEXT& _PER_IO_CONTEXT::operator=(const _PER_IO_CONTEXT rhv)
{
	datalength = rhv.datalength;
	link = rhv.link;
	operateType = rhv.operateType;
	memcpy(&overlap, &(rhv.overlap), sizeof(WSAOVERLAPPED));
	memcpy(&databuf, &(rhv.databuf), sizeof(IO_BUFFER_SIZE));
	return *this;
}

void _PER_IO_CONTEXT::ResetDataBuff()
{
	ZeroMemory(&databuf, sizeof(char) * IO_BUFFER_SIZE);
	datalength = 0;
}

void _PER_IO_CONTEXT::ResetOverlapBuff()
{
	buffer.buf = databuf;
	buffer.len = IO_BUFFER_SIZE;
	ZeroMemory(&overlap, sizeof(WSAOVERLAPPED));
}

void _PER_IO_CONTEXT::ResetBuff()
{
	ResetDataBuff();
	ResetOverlapBuff();
}

#pragma endregion

#pragma region Socket context
_PER_SOCKET_CONTEXT::_PER_SOCKET_CONTEXT() : link(INVALID_SOCKET), storeID(0)
{
	ZeroMemory(&clientAddr, sizeof(SOCKADDR_IN));
	vIoContext.clear();
}

_PER_SOCKET_CONTEXT::~_PER_SOCKET_CONTEXT()
{
	ClearOperate();
	//SAFE_RELEASE_SOCKET(link);
}

_PER_SOCKET_CONTEXT& _PER_SOCKET_CONTEXT::operator=(const _PER_SOCKET_CONTEXT rhv)
{
	link = rhv.link;
	storeID = rhv.storeID;
	memcpy(&clientAddr, &(rhv.clientAddr), sizeof(SOCKADDR_IN));
	int nIoCount = rhv.vIoContext.size();
	for (int i = 0; i < nIoCount; i++)
	{
		if (nullptr != rhv.vIoContext[i])
		{
			LPOPERATE_IO_CONTEXT pTemp = GetNewIoOperate();
			pTemp = rhv.vIoContext[i];
		}
	}

	return *this;
}

void _PER_SOCKET_CONTEXT::ClearOperate()
{
	int nIoCount = vIoContext.size();
	if (nIoCount < 1)
		return;

	for (int i = 0; i < nIoCount; i++)
	{
		if (nullptr != vIoContext[i])
		{
			delete vIoContext[i];
		}

		vIoContext[i] = nullptr;
	}
}

LPOPERATE_IO_CONTEXT _PER_SOCKET_CONTEXT::GetNewIoOperate()
{
	LPOPERATE_IO_CONTEXT p = new _PER_IO_CONTEXT();
	vIoContext.push_back(p);
	return p;
}

void _PER_SOCKET_CONTEXT::RemoveIoOperate(LPOPERATE_IO_CONTEXT pOperate)
{
	if (nullptr == pOperate)
		return;

	std::vector<LPOPERATE_IO_CONTEXT>::iterator iter = vIoContext.begin();
	for (; iter != vIoContext.end(); ++iter)
	{
		if (*iter == pOperate)
		{
			delete pOperate;
			pOperate = nullptr;
			iter = vIoContext.erase(iter);
			return;
		}
	}
}
#pragma endregion

#endif
