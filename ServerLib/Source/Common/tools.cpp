#include "../../Include/Common/tools.h"

std::vector<std::string> SplitString(std::string strString, std::string strParam)
{
	std::vector<std::string> vResult;
	vResult.clear();
	std::string::size_type pos;

	if (strString.length() == 0)
	{
		return vResult;
	}
	strString += strParam;//扩展字符串以方便操作
	int size = (int)strString.size();

	for (int i = 0; i < size; i++)
	{
		pos = strString.find(strParam, i);
		if (pos < size)
		{
			std::string s = strString.substr(i, pos - i);
			vResult.push_back(s);
			i = (int)(pos + strParam.size() - 1);
		}
	}

	return vResult;
}

std::vector<int> SplitStringToInt(std::string strString, std::string strParam)
{
	auto strs = SplitString(strString, strParam);
	auto ints = std::vector<int>();
	for (auto s : strs)
	{
		ints.push_back(atoi(s.c_str()));
	}
	return ints;
}

std::vector<float> SplitStringToFloat(std::string strString, std::string strParam)
{
	auto strs = SplitString(strString, strParam);
	auto ints = std::vector<float>();
	for (auto s : strs)
	{
		ints.push_back((float)atof(s.c_str()));
	}
	return ints;
}

