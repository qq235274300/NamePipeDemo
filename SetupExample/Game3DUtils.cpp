#include "Game3DUtils.h"
#include "PEInfo.h"
#include <filesystem>
#include <iostream>
#include <fstream>
namespace fs = std::filesystem;

std::vector<GameInfo3D> Game3DUtils::ReadAndParseGameData(const std::wstring& filePath)
{
	return ParseGameListJson(ReadJsonFileUtf8(filePath));
}

std::wstring Game3DUtils::Utf8ToWstring(const std::string& str)
{	
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), nullptr, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstrTo[0], size_needed);
	return wstrTo;
}

std::string Game3DUtils::WstringToUtf8(const std::wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &strTo[0], size_needed, nullptr, nullptr);
	return strTo;
}

std::wstring Game3DUtils::GetGame3DConfigPath(const std::wstring& appPath)
{
	//找到真实exe路径
	std::wstring targetPathUnrealEngine = PEInfo::ReadResourceString(appPath, 201);
	if (!targetPathUnrealEngine.empty()) 
	{
		//std::cout << "Found target EXE: " << targetPathUnrealEngine << std::endl;
		std::replace(targetPathUnrealEngine.begin(), targetPathUnrealEngine.end(), '/', '\\');
		targetPathUnrealEngine = appPath.substr(0, appPath.find_last_of(L"\\/") + 1) + targetPathUnrealEngine;
		if (fs::exists(targetPathUnrealEngine))
		{
			std::wcout << "Get Real Config path: " << targetPathUnrealEngine << std::endl;
			
			return targetPathUnrealEngine;
		}
		else
		{
			std::wcout << "Get Config path: " << appPath << std::endl;
			
			return appPath;
		}
	}
	else
	{
		return appPath;
	}

	
}

std::vector<GameInfo3D> Game3DUtils::ParseGameListJson(const std::string& utf8Json)
{
	std::vector<GameInfo3D> gameList;
	json j = json::parse(utf8Json);

	for (const auto& item : j["elementList"])
	{
		if (item.contains("appPath") && item.contains("gameId"))
		{
			GameInfo3D game;
			game.gamePath = Utf8ToWstring(item["appPath"].get<std::string>());
			game.gameId = Utf8ToWstring(item["gameId"].get<std::string>());
			game.gameConfigPath = CheckAndUpdatePath(GetGame3DConfigPath(game.gamePath));
			// 提取可执行文件名
			std::filesystem::path exePath(game.gamePath);
			std::wstring exeName = exePath.stem().wstring();
			game.gameName = exeName;
			if (GameNameSet.find(exeName) != GameNameSet.end())
			{
				game.support3D = true;
			}

			gameList.emplace_back(std::move(game));
		}
	}
	return gameList;
}

std::string Game3DUtils::PatchSupportGame3D(const std::string& inputJsonUtf8)
{
	//read json from somewhere
	
	json root = json::parse(inputJsonUtf8);

	for (auto& element : root["elementList"])
	{
		if (!element.contains("appPath"))
			continue;

		std::wstring appPath = Utf8ToWstring(element["appPath"].get<std::string>());
		std::wstring folderName = std::filesystem::path(appPath).filename().wstring();

		if (GameNameSet.find(folderName) != GameNameSet.end())
		{
			element["support_3D"] = true;
		}
	}

	return root.dump(); // 返回修改后的 JSON（UTF-8 字符串）
}

std::string Game3DUtils::ReadJsonFileUtf8(const std::wstring& filePath)
{
	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file)
	{
		return "";
	}
		
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

std::wstring Game3DUtils::CheckAndUpdatePath(const std::wstring& targetPath)
{
	std::string basePath;         // UTF-8 字符串（可用于 JSON 或日志）
	std::wstring baseFilePath;

	fs::path fullPath(targetPath);
	fs::path parent = fullPath.parent_path();

	basePath = parent.string();
	baseFilePath = parent;

	wchar_t windowsPath[MAX_PATH];
	GetWindowsDirectoryW(windowsPath, MAX_PATH);

	if (baseFilePath.find(windowsPath) == 0)
	{
		std::cout << "Installation to the Windows directory is prohibited!" << std::endl;
		return L"";
	}
	
	std::wcout << "Base File Path: " << baseFilePath << std::endl;
	return baseFilePath;
}

void Game3DUtils::Copy3DConfigToGame(const std::filesystem::path& Config3DPath, const std::wstring& gameId,const std::vector<GameInfo3D>& GameInfos)
{
	std::wstring gamePath = L"";
	for (int i = 0; i < GameInfos.size(); ++i)
	{
		if (GameInfos[i].gameId == gameId)
		{
			gamePath = GameInfos[i].gameConfigPath;
			break;
		}
	}

	if (gamePath.empty() || !fs::exists(gamePath))
	{
		std::wcout << L"[Error] Game config path 不存在: " << gamePath << std::endl;
		return;
	}

	// 迭代源目录中的所有内容
	for (const auto& entry : fs::recursive_directory_iterator(Config3DPath))
	{
		const fs::path& srcPath = entry.path();
		fs::path relativePath = fs::relative(srcPath, Config3DPath);
		fs::path destPath = fs::path(gamePath) / relativePath;

		if (entry.is_directory())
		{
			fs::create_directories(destPath);
		}
		else if (entry.is_regular_file())
		{
			fs::create_directories(destPath.parent_path()); // 确保父目录存在
			fs::copy_file(srcPath, destPath, fs::copy_options::overwrite_existing);
			std::wcout << L"[Copied] " << srcPath.wstring() << L" " << destPath.wstring() << std::endl;
		}
	}
}
