#pragma once
#include <filesystem>
#include <string>

struct InstallInfo
{
	bool is64Bit = false;
	std::string targetPath, targetName; //exeÖ´ÐÐ Ä¿Â¼
	std::string basePath; //¿½±´Ä¿Â¼
	std::filesystem::path baseFilePath;
	std::string modulePath, configPath, presetPath;
};

