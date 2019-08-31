
#ifndef __TYPE_DEFINES_H__
#define __TYPE_DEFINES_H__


#pragma region data type
typedef long long int64;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef int int32;
typedef unsigned short uint16;
#pragma endregion

#pragma region Thread status
enum EServerThreadStatusType
{
	ESTST_NONE = 0,				//	无
	ESTST_INITIALIZED,			//	初始化
	ESTST_START,				//	开始
	ESTST_RUNNING,				//	执行
	ESTST_SLEPT,				//	休眠
	ESTST_DESTROIED,			//	销毁

	ESTST_MAX
};
#pragma endregion

#pragma region Unlock queue about
enum EQueueElementStatusType
{
	EQEST_NONE = 0,			//	无效状态
	EQEST_NODATA,			//	没有数据
	EQEST_WAIT,				//	等待入队
	EQEST_IN,				//	进入队列
	EQEST_OUT,				//	已出队列

	EQEST_MAX
};

enum EQueueOperateResultType
{
	EQORT_SUCCESS = 0,					//	操作成功
	EQORT_PUSH_INVALID_ELEMENT,			//	添加错误：无效的元素
	EQORT_PUSH_FULL_QUEUE,				//	添加错误：队列已满
	EQORT_POP_EMPTY_QUEUE,				//	弹出错误：空队列
	EQORT_POP_INVALID_ELEMENT,			//	弹出错误：无效的元素

	EQORT_MAX
};

enum EStandQueueType
{
	ESQT_NONE = 0,			//	无
	ESQT_READ_QUEUE,		//	读取队列
	ESQT_WRITE_QUEUE,		//	写入队列
	ESQT_LOG_QUEUE,			//	日志队列

	ESQT_MAX
};
#pragma endregion 

#pragma region Log about
enum ELogLevelType
{
	ELLT_NONE = 0,		//	无
	ELLT_ECHO,			//	普通
	ELLT_DEBUG,			//	调试（此级别及以下，非DEBUG模式不做文件打印）
	ELLT_MSG,			//	信息
	ELLT_WARNING,		//	警告
	ELLT_ERROR,			//	错误

	ELLT_MAX
};

#pragma endregion

#pragma region File check permission

enum EFilePermissionCheckResult
{
	EFPCR_SUCCESS = 0,			//	有该权限
	EFPCR_NO_PERMISSION			//	没有该权限
};

enum EFileCheckSystemType
{
	EFCST_EXISTS = 0,			//	查看文件是否存在
	EFCST_WRITE = 2,			//	当前用户是否有写权限
	EFCST_READ = 4,				//	当前用户是否有读权限
	EFCST_READ_WRITE = 6		//	当前用户是否有读写权限
};

enum EFileCheckType
{
	EFCT_NONE = 0,
	EFCT_EXITS = 1,
	EFCT_WRITE = 2,
	EFCT_READ = 4,
};

enum EFileDeleteErrorType
{
	EFDET_SUCCESS = 0,			//	删除文件成功
	EFDET_EROFS,				//	文件为只读权限
	EFDET_EFAULT,				//	文件名称指针超出可读取内存空间
	EFDET_ENAMETOOLONG,			//	文件名超长
	EFDET_ENOMEN,				//	内存不足
	EFDET_ELOOP,				//	文件有过多的符号连接
	EFDET_EIO,					//	IO错误

	EFDET_MAX
};

#pragma endregion

#endif	//	__TYPE_DEFINES_H__
