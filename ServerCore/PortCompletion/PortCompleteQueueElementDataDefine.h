

#ifndef __PORT_COMPLETE_QUEUQ_ELEMENT_DATA_DEFINE_H__
#define __PORT_COMPLETE_QUEUQ_ELEMENT_DATA_DEFINE_H__
#include "../../ServerLib/Include/Common/UnLockElementTypeDefine.h"

enum ElementSocketDataGroupType
{
	EESDGT_NONE					= 0,
	EESDGT_REGISTER				= EEDGT_SOCKET << 24 + 1,			//	注册数据
	EESDGT_MESSAGE_SEND			= EEDGT_SOCKET << 24 + 2,			//	消息发送数据
	EESDGT_MSG_SEND_OVER		= EEDGT_SOCKET << 24 + 3,			//	发送消息结束
	EESDGT_MESSAGE_RECV			= EEDGT_SOCKET << 24 + 4,			//	接收消息开始
	EESDGT_MSG_RECV_OVER		= EEDGT_SOCKET << 24 + 5,			//	接收消息结束

	EESDGT_MAX
};

//	工作线程功能标记	
enum PortCompletionThreadFunctionMask
{
	EPCTFT_NONE					= 0,
	EPCTFT_RECV					= 1,
	EPCTFT_SEND					= 2,
	EPCTFT_LISTEN				= 3,
};

//	注册网络连接类型
enum PortCompletionSocketRegisterType
{
	EPCSRT_NONE		= 0,		//	无
	EPCSRT_STORE,				//	存储
	EPCSRT_RECV,				//	收消息
	EPCSRT_SEND,				//	发消息

	EPCSRT_MAX
};

//	完成端口操作类型
enum ECompletionPortOperateType
{
	ECPOT_NONE = 0,			//	不可用
	ECPOT_ACCEPT,			//	链接操作			
	ECPOT_WAIT,				//	等待操作
	ECPOT_RECIVE,			//	接受操作
	ECPOT_SEND,				//	发送操作

	ECPOT_MAX
};

enum ESocketLinkType
{
	ESLT_NONE = 0,
	ESLT_CLIENT_SERVER,		//	客户端到服务器
	ESLT_SERVER_SERVER,		//	服务器到服务器
	ESLT_SERVER_CLIENT,		//	服务器与客户端

	ESLT_MAX
};

enum ESocketLinkStatusType
{
	ESLST_NONE = 0,
	ESLST_ACCEPT,
	ESLST_WAIT,
	ESLST_RECV,
	ESLST_SEND,

	ESLST_MAX
};

#define LISTEN_LINK_COUNT 4096
#define IO_BUFFER_SIZE 1024







#endif	//	__PORT_COMPLETE_QUEUQ_ELEMENT_DATA_DEFINE_H__
