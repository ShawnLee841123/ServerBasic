
#include "../../Include/Common/UnLockQueue.h"
#include <stdlib.h>
#include <string>

#pragma region Queue element base
UnLockQueueElementBase::UnLockQueueElementBase() : m_pData(nullptr), m_eStatus(EQEST_NONE), m_uDataSize(0)
{}

UnLockQueueElementBase::~UnLockQueueElementBase()
{
	ClearElement();
}

void* UnLockQueueElementBase::GetData()
{
	//	一旦数据被取出，就只能等待数据被销毁。
	m_eStatus = EQEST_OUT;
	//void* pData = malloc(uSize);
	//memcpy(pData, m_pData, uSize);
	return m_pData;
}

EQueueElementStatusType UnLockQueueElementBase::GetCurStatus()
{
	return m_eStatus;
}

bool UnLockQueueElementBase::Enable()
{
	return (nullptr != m_pData) && (EQEST_IN == m_eStatus);
}

bool UnLockQueueElementBase::SetData(void* pData, uint32 uSize)
{
	if (nullptr == pData)
		return false;

	if (m_eStatus > EQEST_NODATA)
		return false;

	if (uSize <= 0)
		return false;

	m_pData = malloc(uSize);
	memcpy(m_pData, pData, uSize);
	m_uDataSize = uSize;
	m_eStatus = EQEST_WAIT;
	return true;
}

void UnLockQueueElementBase::ClearElement()
{
	//	malloc申请的内存，需要free释放
	if (nullptr != m_pData)
		free(m_pData);

	m_pData = nullptr;
	m_eStatus = EQEST_NODATA;
	m_uDataSize = 0;
}

void UnLockQueueElementBase::OnInQueue()
{
	m_eStatus = EQEST_IN;
}

uint32 UnLockQueueElementBase::GetDataSize()
{
	return m_uDataSize;
}

#pragma region operator override
UnLockQueueElementBase& UnLockQueueElementBase::operator=(const UnLockQueueElementBase rhv)
{
	m_eStatus = rhv.m_eStatus;
	if (rhv.m_uDataSize > 0)
	{
		//	简单的将内存指向同一个地址是不行的，有一个释放了就全没了。
		//	这里申请了内存。都是使用void*的形式进行的存储。只好使用malloc来分配内存
		m_pData = malloc(rhv.m_uDataSize);
		memcpy(m_pData, rhv.m_pData, rhv.m_uDataSize);
		m_uDataSize = rhv.m_uDataSize;
	}
	
	return *this;
}

#pragma endregion

#pragma endregion

#pragma region Data Element Base
UnLockQueueDataElementBase::UnLockQueueDataElementBase() : m_pEleData(nullptr)
{
}
UnLockQueueDataElementBase::~UnLockQueueDataElementBase()
{
}
#pragma region parent override
void* UnLockQueueDataElementBase::GetData()
{
	m_eStatus = EQEST_OUT;
	return m_pEleData;
}

bool UnLockQueueDataElementBase::SetData(UnLockQueueElementDataBase* pData, uint32 uSize)
{
	if (nullptr == pData)
		return false;

	if (m_eStatus > EQEST_NODATA)
		return false;

	if (uSize <= 0)
		return false;

	m_pEleData = (UnLockQueueElementDataBase*)malloc(uSize);
	memcpy(m_pEleData, pData, uSize);
	m_uDataSize = uSize;
	m_eStatus = EQEST_WAIT;
	return true;
}

void UnLockQueueDataElementBase::ClearElement()
{
	//	malloc申请的内存，需要free释放
	if (nullptr != m_pEleData)
		free(m_pEleData);

	m_pEleData = nullptr;
	m_eStatus = EQEST_NODATA;
	m_uDataSize = 0;
}

bool UnLockQueueDataElementBase::Enable()
{
	return (nullptr != m_pEleData) && (EQEST_IN == m_eStatus);
}

uint32 UnLockQueueDataElementBase::GetDataID()
{
	return (nullptr != m_pEleData) ? m_pEleData->GetDataID() : 0;
}

UnLockQueueDataElementBase& UnLockQueueDataElementBase::operator= (const UnLockQueueDataElementBase rhv)
{
	m_eStatus = rhv.m_eStatus;
	if (rhv.m_uDataSize > 0)
	{
		//	简单的将内存指向同一个地址是不行的，有一个释放了就全没了。
		//	这里申请了内存。都是使用void*的形式进行的存储。只好使用malloc来分配内存
		m_pEleData = (UnLockQueueElementDataBase*)malloc(rhv.m_uDataSize);
		memcpy(m_pEleData, rhv.m_pEleData, rhv.m_uDataSize);
		m_uDataSize = rhv.m_uDataSize;
	}

	return *this;
}
#pragma endregion

#pragma endregion

#pragma region Queue base
UnLockQueueBase::UnLockQueueBase() : m_nHead(0), m_nTail(0), m_uElementCount(0)
{
}

UnLockQueueBase::~UnLockQueueBase()
{
	Destroy();
}

EQueueOperateResultType UnLockQueueBase::PushQueueElement(UnLockQueueElementBase* pElement)
{
	if (nullptr == pElement)
		return EQORT_PUSH_INVALID_ELEMENT;

	if (pElement->GetCurStatus() <= EQEST_NODATA)
		return EQORT_PUSH_INVALID_ELEMENT;

	if (m_nHead == ((m_nTail + 1) % QUEUE_COUNT))
		return EQORT_PUSH_FULL_QUEUE;

	m_arrData[m_nTail] = pElement;
	pElement->OnInQueue();
	m_nTail = (m_nTail + 1) % QUEUE_COUNT;
	m_uElementCount++;
	return EQORT_SUCCESS;
}

EQueueOperateResultType UnLockQueueBase::PushQueueElement(UnLockQueueElementDataBase* pData, uint32 uSize)
{
	UnLockQueueDataElementBase* pElement = new UnLockQueueDataElementBase();
	pElement->SetData(pData, uSize);
	return PushQueueElement(pElement);
}

//EQueueOperateResultType UnLockQueueBase::PushQueueElement(void* pData, uint32 uSize)
//{
//	UnLockQueueElementBase* pElement = new UnLockQueueElementBase();
//	pElement->SetData(pData, uSize);
//	return PushQueueElement(pElement);
//}

UnLockQueueElementBase* UnLockQueueBase::PopQueueElement(EQueueOperateResultType& eRet)
{
	if (m_uElementCount == 0)
	{
		eRet = EQORT_POP_EMPTY_QUEUE;
		return nullptr;
	}

	if (m_nHead == m_nTail)
	{
		eRet = EQORT_POP_EMPTY_QUEUE;
		return nullptr;
	}

	UnLockQueueElementBase* pTemp = m_arrData[m_nHead];
	m_nHead = (m_nHead + 1) % QUEUE_COUNT;
	eRet = EQORT_SUCCESS;
	m_uElementCount--;
	return pTemp;
}

void UnLockQueueBase::Destroy()
{
	m_nHead = 0;
	m_nTail = 0;
	m_uElementCount = 0;
	memset(m_arrData, 0, sizeof(UnLockQueueElementBase) * QUEUE_COUNT);
}
#pragma endregion