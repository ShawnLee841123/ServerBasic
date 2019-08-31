
#ifndef __UNLOCK_ELEMENT_TYPE_DEFINE_H__
#define __UNLOCK_ELEMENT_TYPE_DEFINE_H__

enum ElementDataGroupType
{
	EEDGT_NONE = 0,
	EEDGT_LOG,				//	日志组
	EEDGT_THREAD,			//	线程组
	EEDGT_SOCKET,			//	链接组

	EEDGT_MAX
};

enum ElementLogDataGroupType
{
	EELDGT_NONE			= 0,
	EELDGT_PRINT		= EEDGT_LOG << 24 + 1,			//	打印日志
	EELDGT_REGISTER		= EEDGT_LOG << 24 + 2,		//	注册日志队列

	EELDGT_MAX
};


#endif	//	__UNLOCK_ELEMENT_TYPE_DEFINE_H__
