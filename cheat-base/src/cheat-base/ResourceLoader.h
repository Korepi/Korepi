#pragma once
class ResourceLoader
{
public:
	static std::string Load(const char* name, const char* type);
	static std::string Load(int resID, const char* type);

	static bool LoadEx(const char* name, const char* type, LPBYTE& pDest, DWORD& size);
	static bool LoadEx(int resId, const char* type, LPBYTE& pDest, DWORD& size);

	static void SetModuleHandle(HMODULE handle);

private:
	inline static HMODULE s_Handle = nullptr;
};

