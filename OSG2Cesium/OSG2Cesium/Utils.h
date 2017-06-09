#pragma once
#include <string>
#include <vector>
class Utils
{
public: 
	Utils();
	~Utils();
	//列出所有子文件夹
	static std::vector<std::string> findSubdirs(std::string dir);
	//文件路径分割
	static void splitFilepath(std::string filepath, std::string& dir, std::string& name, std::string& ext);
};

