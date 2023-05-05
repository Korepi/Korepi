#include "pch.h"
#include "ResourceLoader.h"
#include "util.h"

std::string ResourceLoader::Load(const char* name, const char* type)
{
	LPBYTE pData = nullptr;
	DWORD size = 0;
	if (!LoadEx(name, type, pData, size))
	{
		LOG_LAST_ERROR("Failed to load resource %s", name);
		return {};
	}

	return std::string(reinterpret_cast<char*>(pData), size);
}

std::string ResourceLoader::Load(int resID, const char* type)
{
	return ResourceLoader::Load(MAKEINTRESOURCE(resID), type);
}

bool ResourceLoader::LoadEx(const char* name, const char* type, LPBYTE& pDest, DWORD& size)
{
	if (s_Handle == nullptr)
		return false;

	HRSRC hResource = FindResource(s_Handle, name, type);
	if (hResource) {
		HGLOBAL hGlob = LoadResource(s_Handle, hResource);
		if (hGlob) {
			size = SizeofResource(s_Handle, hResource);
			pDest = static_cast<LPBYTE>(LockResource(hGlob));
			if (size > 0 && pDest)
				return true;
		}
	}
	return false;
}

bool ResourceLoader::LoadEx(int resId, const char* type, LPBYTE& pDest, DWORD& size)
{
	return ResourceLoader::LoadEx(MAKEINTRESOURCE(resId), type, pDest, size);
}

void ResourceLoader::SetModuleHandle(HMODULE handle)
{
	s_Handle = handle;
}
