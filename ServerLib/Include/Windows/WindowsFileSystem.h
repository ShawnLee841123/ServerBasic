

#ifdef _WIN_

#ifndef __WINDOWS_FILE_SYSTEM_H__
#define __WINDOWS_FILE_SYSTEM_H__
#include "../Common/TypeDefines.h"
#include <stdio.h>
//	获取当前工作路径
bool Windows_GetCurrentDir(char* strOut, uint32 strCount);

//	创建目录
bool Windows_CreatePath(const char* strName);

//	删除目录
bool Windows_DeletePath(const char* strName);

//	文件是否存在
bool Windows_FileExists(const char* strFileName);

//	目录是否存在
bool Windows_PathExists(const char* strName);

//	创建文件(未实现)
FILE* Windows_CreateFile(const char* strFileName);

//	删除文件
bool Windows_DeleteFile(const char* strFileName);

//	检查文件或目录权限(非公开调用)
EFilePermissionCheckResult Windows_CheckFileOrPathPermission(const char* strName, EFileCheckSystemType eType);

//	检查文件或目录是否有权限（公开）
//	参数strName问路径时，只能检查路径是否存在
EFilePermissionCheckResult Windows_CheckFilePermission(const char* strName, int eType);

void Windows_PrintLogTextToScreen(const char* strValue, void* pConsole, ELogLevelType eType);
#endif	//	__WINDOWS_FILE_SYSTEM_H__

#endif	//	_WINDOWS_


