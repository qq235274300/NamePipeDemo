#pragma once
#include <filesystem>
#include <string>

struct InstallInfo
{
	bool is64Bit = false;
	std::string targetPath, targetName; //exeִ�� Ŀ¼
	std::string basePath; //����Ŀ¼
	std::filesystem::path baseFilePath;
	std::string modulePath, configPath, presetPath;
};

