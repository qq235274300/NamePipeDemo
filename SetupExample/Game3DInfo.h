#pragma once
#include <Windows.h>
#include <nlohmann/json.hpp>
#include <stringapiset.h>
#include <string>
#include <unordered_set>

//MSVC 工程配置为 Use Unicode Character Set，默认 TCHAR = wchar_t，字符串用 wchar_t* 或 std::wstring 表示。
//接口传参、结构字段、路径等优先用 wchar_t 或 std::wstring 表示，编码兼容 Windows 的宽字符 API。

using json = nlohmann::json;

struct GameInfo3D
{	
	std::wstring gameName;
	std::wstring gameId;
	std::wstring gamePath;   //应该为游戏路径带exe(当前没带exe)
	std::wstring gameConfigPath;   //配置文件安装路径
	bool support3D = false;
	
};

static const std::unordered_set<std::wstring> GameNameSet = {
	L"God of War Ragnarok",
	L"BlackMythWukong",
	L"P5R",
	L"Rise of the Tomb Raider",
	L"Sekiro",
	L"Shadow of the Tomb Raider"
};




