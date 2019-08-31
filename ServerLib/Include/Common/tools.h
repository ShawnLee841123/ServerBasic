

#ifndef __PUBLIC_TOOLS_H__
#define __PUBLIC_TOOLS_H__

#include <vector>
#include <string>

std::vector<std::string> SplitString(std::string strString, std::string strParam);
std::vector<int> SplitStringToInt(std::string strString, std::string strParam);
std::vector<float> SplitStringToFloat(std::string strString, std::string strParam);
#endif //__PUBLIC_TOOLS_H__

