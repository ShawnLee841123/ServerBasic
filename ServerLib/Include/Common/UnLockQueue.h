

#ifndef __UNLOCK_QUEUE_H__
#define __UNLOCK_QUEUE_H__

#include "TypeDefines.h"

#define QUEUE_COUNT	2048

#pragma region Queue element base
class UnLockQueueElementBase
{
public:
	UnLockQueueElementBase();
	virtual ~UnLockQueueElementBase();

	//	获取数据（一旦获取数据，则数据为不可用状态，只能清除）
	virtual void* GetData();
	//	获取当前状态
	virtual EQueueElementStatusType GetCurStatus();
	//	是否可用
	virtual bool Enable();
	//	设置数据（如果已有数据，则不可以设置）
	virtual bool SetData(void* pData, uint32 uSize);
	//	考虑到复用，增加的接口
	virtual void ClearElement();
	virtual void OnInQueue();
	virtual uint32 GetDataSize();
#pragma region operator override
	UnLockQueueElementBase& operator= (const UnLockQueueElementBase rhv);
#pragma endregion

protected:
	void*						m_pData;			//	数据
	EQueueElementStatusType		m_eStatus;			//	元素状态
	uint32						m_uDataSize;		//	数据大小
};

#pragma endregion

//	下列两个结构可任意取一个集成
#pragma region Element Data Base
class UnLockQueueElementDataBase
{
public:
	UnLockQueueElementDataBase(): DataID(0){}
	virtual ~UnLockQueueElementDataBase(){}
	virtual UnLockQueueElementDataBase& operator= (const UnLockQueueElementDataBase rhv) { DataID = rhv.DataID; return *this; }
	virtual uint32 GetDataID() const { return DataID; }
protected:
	uint32 DataID;
};
#pragma endregion


#pragma region Data Element Base
class UnLockQueueDataElementBase : public UnLockQueueElementBase
{
public:
	UnLockQueueDataElementBase();
	virtual ~UnLockQueueDataElementBase();
	
#pragma region parent override
	virtual void* GetData() override;
	virtual bool SetData(UnLockQueueElementDataBase* pData, uint32 uSize);
	virtual void ClearElement() override;
	virtual bool Enable() override;
	virtual uint32 GetDataID();
	virtual UnLockQueueDataElementBase& operator= (const UnLockQueueDataElementBase rhv);
#pragma endregion
protected:
	UnLockQueueElementDataBase*				m_pEleData;
};
#pragma endregion


/*

无锁队列的释放，交给写线程，读线程无权对写线程进行

*/

#pragma region Queue base

class UnLockQueueBase
{
public:
	UnLockQueueBase();
	virtual ~UnLockQueueBase();

	//	添加队列元素（该元素的数据内容可以为局域变量）
	virtual EQueueOperateResultType PushQueueElement(UnLockQueueElementBase* pElement);
	//	添加队列元素
	virtual EQueueOperateResultType PushQueueElement(UnLockQueueElementDataBase* pData, uint32 uSize);
	////	添加队列元素（元素内容只能是new出来的指针）
	//virtual EQueueOperateResultType PushQueueElement(void* pData, uint32 uSize);

	virtual UnLockQueueElementBase* PopQueueElement(EQueueOperateResultType& eRet);

	virtual void Destroy();
protected:

	int32											m_nHead;						//	头
	int32											m_nTail;						//	尾
	UnLockQueueElementBase*							m_arrData[QUEUE_COUNT];			//	数据
	uint32											m_uElementCount;				//	当前数量
};
#pragma endregion

#endif //	__UNLOCK_QUEUE_H__
