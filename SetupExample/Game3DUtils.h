#pragma once
#include "Game3DInfo.h"



class Game3DUtils
{
public:
	static std::vector<GameInfo3D> ReadAndParseGameData(const std::wstring& filePath);

private:
	//****************************Json*****************************//
	static std::wstring Utf8ToWstring(const std::string& str);
	
	static std::string WstringToUtf8(const std::wstring& wstr);
	
	static std::wstring GetGame3DConfigPath(const std::wstring& appPath);
	
	static std::vector<GameInfo3D> ParseGameListJson(const std::string& utf8Json);

	static std::string PatchSupportGame3D(const std::string& inputJsonUtf8);
	//从本地读取json文件
	static std::string ReadJsonFileUtf8(const std::wstring& filePath);

	static std::wstring CheckAndUpdatePath(const std::wstring& targetPath);
	
	//****************************Copy & Delete 3D Config*****************************//
public:
	static void Copy3DConfigToGame(const std::filesystem::path& Config3DPath, const std::wstring& gameId, const std::vector<GameInfo3D>& GameInfos);
	
public:


private:
	
	

};

