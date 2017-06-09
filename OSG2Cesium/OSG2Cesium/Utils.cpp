#include "Utils.h"
#include <Windows.h>
 

Utils::Utils()
{
}


Utils::~Utils()
{
}
void Utils::splitFilepath(std::string filepath, std::string& dir, std::string& name, std::string& ext)
{
	int lastdot = -1;
	int lastslash = -1;
	int secondslash = -1;
	for (int i = filepath.size() - 1; i >= 0; i--)
	{
		if (lastslash == -1 && (filepath[i] == '/' || filepath[i] == '\\'))
		{
			lastslash = i;
			//break;
		}
		if (lastslash != -1 && secondslash == -1 && (filepath[i] == '/' || filepath[i] == '\\'))
		{
			secondslash = i;
			//break;
		}
		if (lastdot == -1 && filepath[i] == '.')
		{
			lastdot = i;
		}
	}
	ext = filepath.substr(lastdot, filepath.size() - lastdot);
	name = filepath.substr(lastslash + 1, filepath.size() - (lastslash + 1) - ext.size());
	dir = filepath.substr(0, secondslash + 1);

}
std::vector<std::string> Utils::findSubdirs(std::string dir)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	std::string sPath;
	std::vector<std::string> MyVect;//"C:\\Documents and Settings\\yugesh\\Desktop\\*"
	sPath.assign(dir);
	hFind = FindFirstFileA(sPath.data(), &FindFileData);
	do
	{
		if (FindFileData.dwFileAttributes == 16)
		{
			if (std::string(FindFileData.cFileName) != "." && std::string(FindFileData.cFileName) != "..")
			{
				MyVect.push_back(FindFileData.cFileName);
			}

		}
	} while (FindNextFileA(hFind, &FindFileData));
	FindClose(hFind);
	//for (int i = 0; i<MyVect.size(); i++)
	//	cout << MyVect.at(i).data() << endl;
	return MyVect;
}