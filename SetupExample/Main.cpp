#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>
#include "PEInfo.h"
#include "InstallInfo.h"
#include "Game3DUtils.h"

namespace fs = std::filesystem;



void CopyDirectoryToBaseDirectory(const fs::path& source, InstallInfo& installInfo)
{
	try
	{
		// ����Ŀ��Ŀ¼
		if (!fs::exists(installInfo.baseFilePath))
		{
			std::cout << "Base Path not exist!" << std::endl;
			return;
		}
		// ����ԴĿ¼�е���������
		for (const auto& entry : fs::recursive_directory_iterator(source))
		{
			const auto& path = entry.path();
			auto relativePathStr = path.lexically_relative(source).string();
			fs::path targetPath = installInfo.baseFilePath / relativePathStr;

			if (fs::is_directory(path))
			{
				fs::create_directories(targetPath);
			}
			else if (fs::is_regular_file(path))
			{
				// �����ļ����滻�Ѵ��ڵ��ļ�
				fs::copy_file(path, targetPath, fs::copy_options::overwrite_existing);
			}
		}
	}
	catch (const fs::filesystem_error& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}



int main()
{
	std::vector<GameInfo3D> gameList;

	std::wstring jsonFilePath = L"C:\\Users\\ChiliShao\\source\\repos\\ExampleApp\\3dexample\\SetupExample\\gamejson_test.txt";
	gameList = Game3DUtils::ReadAndParseGameData(jsonFilePath);

	for (int i = 0; i < gameList.size(); ++i)
	{
		std::wcout << L" exeName: " << gameList[i].gameName
			<< L"support3D: " << (gameList[i].support3D ? L"true" : L"false") << std::endl;
	}
	

	while (true)
	{

	}
	InstallInfo installInfo;


	fs::path sourceDir;
	//�����滻3D�����ļ�
	CopyDirectoryToBaseDirectory(sourceDir, installInfo);


	return 0;
}