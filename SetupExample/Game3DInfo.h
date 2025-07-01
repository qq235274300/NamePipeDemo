#pragma once
#include <Windows.h>
#include <nlohmann/json.hpp>
#include <stringapiset.h>
#include <string>
#include <unordered_set>

//MSVC ��������Ϊ Use Unicode Character Set��Ĭ�� TCHAR = wchar_t���ַ����� wchar_t* �� std::wstring ��ʾ��
//�ӿڴ��Ρ��ṹ�ֶΡ�·���������� wchar_t �� std::wstring ��ʾ��������� Windows �Ŀ��ַ� API��

using json = nlohmann::json;

struct GameInfo3D
{	
	std::wstring gameName;
	std::wstring gameId;
	std::wstring gamePath;   //Ӧ��Ϊ��Ϸ·����exe(��ǰû��exe)
	std::wstring gameConfigPath;   //�����ļ���װ·��
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




