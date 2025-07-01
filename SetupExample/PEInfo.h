#pragma once
#include <windows.h>
#include <imagehlp.h>
#include <string>
#include <vector>

#pragma comment(lib, "ImageHlp.lib")

class PEInfo
{
public:
	enum class BinaryType : WORD
	{
		MACHINE_UNKNOWN = 0x0,
		MACHINE_I386 = 0x14c,
		MACHINE_AMD64 = 0x8664,
	};

	enum class ImageDirectory : WORD
	{
		ENTRY_EXPORT = 0,
		ENTRY_IMPORT = 1,
	};

	PEInfo(const std::string& path)
	{
		LOADED_IMAGE image;
		if (MapAndLoad(path.c_str(), nullptr, &image, FALSE, TRUE))
		{
			auto imports = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(ImageDirectoryEntryToData(image.MappedAddress, FALSE, static_cast<DWORD>(ImageDirectory::ENTRY_IMPORT), &size));

			if (imports != nullptr)
			{
				while (imports->OriginalFirstThunk != 0)
				{
					auto module = reinterpret_cast<char*>(ImageRvaToVa(reinterpret_cast<IMAGE_NT_HEADERS*>(image.FileHeader), image.MappedAddress, imports->Name, nullptr));
					if (module != nullptr)
					{
						modules.push_back(module);
					}

					++imports;
				}
			}

			type = static_cast<BinaryType>(reinterpret_cast<IMAGE_NT_HEADERS*>(image.FileHeader)->FileHeader.Machine);

			UnMapAndLoad(&image);
		}
	}

	BinaryType GetType() const
	{
		return type;
	}

	const std::vector<std::string>& GetModules() const
	{
		return modules;
	}

	static std::wstring ReadResourceString(const std::wstring& path, WORD id)
	{
		std::wstring result;

		// 加载资源 DLL 或 EXE
		HMODULE module = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
		if (module != nullptr)
		{
			// 查找资源：ID + 类型为 RCDATA
			HRSRC info = FindResourceW(module, MAKEINTRESOURCEW(id), RT_RCDATA);
			if (info != nullptr)
			{
				HGLOBAL handle = LoadResource(module, info);
				if (handle != nullptr)
				{
					auto data = static_cast<const wchar_t*>(LockResource(handle));
					DWORD dataSize = SizeofResource(module, info);

					// 转换为 std::wstring
					if (data && dataSize > 0)
					{
						result.assign(data, data + (dataSize / sizeof(wchar_t)));
					}
				}
			}

			FreeLibrary(module);
		}

		return result;
	}

private:
	BinaryType type;
	std::vector<std::string> modules;
	DWORD size;
};

