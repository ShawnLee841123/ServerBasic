
#include "../../Include/System/FileSystem.h"

#ifdef _WIN_
#include "../../Include/Windows/WindowsFileSystem.h"
#else

#endif

//	获取当前工作路径
bool GetCurrentDir(char* strOut, uint32 strCount)
{
	bool bRet = false;
#ifdef _WIN_
	bRet = Windows_GetCurrentDir(strOut, strCount);
#else
#endif
	return bRet;
}

//	创建目录
bool CreatePath(const char* strName)
{
	bool bRet = false;
#ifdef _WIN_
	bRet = Windows_CreatePath(strName);
#else
#endif
	return bRet;
}

//	删除目录
bool DeletePath(const char* strName)
{
	bool bRet = false;
#ifdef _WIN_
	bRet = Windows_DeletePath(strName);
#else
#endif
	return bRet;
}

//	文件是否存在
bool FileExists(const char* strFileName)
{
	bool bRet = false;
#ifdef _WIN_
	bRet = Windows_FileExists(strFileName);
#else
#endif
	return bRet;
}

//	目录是否存在
bool PathExists(const char* strName)
{
	bool bRet = false;
#ifdef _WIN_
	bRet = Windows_PathExists(strName);
#else
#endif
	return bRet;
}

//	创建文件(未实现)
FILE* System_CreateFile(const char* strFileName)
{
	FILE* pFile = nullptr;
#ifdef _WIN_
	pFile = Windows_CreateFile(strFileName);
#else
#endif

	return pFile;
}

//	删除文件
bool DeleteFile(const char* strFileName)
{
	bool bRet = false;
#ifdef _WIN_
	bRet = Windows_DeleteFile(strFileName);
#else
#endif
	return bRet;
}

//	检查文件或目录权限(非公开调用)
EFilePermissionCheckResult CheckFileOrPathPermission(const char* strName, EFileCheckSystemType eType)
{
	EFilePermissionCheckResult eRet = EFPCR_NO_PERMISSION;
#ifdef _WIN_
	eRet = Windows_CheckFileOrPathPermission(strName, eType);
#else
#endif
	return eRet;
}

//	检查文件或目录是否有权限（公开）
//	参数strName问路径时，只能检查路径是否存在
EFilePermissionCheckResult CheckFilePermission(const char* strName, int eType)
{
	EFilePermissionCheckResult eRet = EFPCR_NO_PERMISSION;
#ifdef _WIN_
	eRet = Windows_CheckFilePermission(strName, eType);
#else
#endif

	return eRet;
}

void PrintLogTextToScreen(const char* strValue, void* pConsole, ELogLevelType eType)
{
#ifdef _WIN_
	Windows_PrintLogTextToScreen(strValue, pConsole, eType);
#else
#endif
}