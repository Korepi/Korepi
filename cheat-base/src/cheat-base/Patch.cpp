#include <pch.h>
#include "Patch.h"

#include <cheat-base/util.h>

bool Patch::Install(uint64_t address, std::vector<uint8_t> value)
{
	if (patches.count(address) > 0)
	{
		LOG_ERROR("Failed to install patch: patch already installed.");
		return false;
	}

	auto oldValue = WriteMemory(address, value);
	if (oldValue == nullptr)
		return false;

	patches[address] = oldValue;
	return true;
}

bool Patch::Restore(uint64_t address)
{
	if (patches.count(address) == 0)
	{
		LOG_ERROR("Failed to restore patch: not found patch to target address 0x%016X", address);
		return false;
	}

	auto restoreValue = patches[address];
	auto oldValue = WriteMemory(address, *restoreValue);
	if (oldValue == nullptr)
		return false;

	patches.erase(address);
	delete restoreValue;
	delete oldValue;

	return true;
}

std::vector<uint8_t>* Patch::WriteMemory(uint64_t address, std::vector<uint8_t> value) 
{
	MEMORY_BASIC_INFORMATION information{};
	auto size = VirtualQuery(reinterpret_cast<void*>(address), &information, sizeof(information));
	if (size < sizeof(information))
	{
		LOG_LAST_ERROR("Failed to get page information");
		return nullptr;
	}

	if (information.State != MEM_COMMIT) 
	{
		LOG_ERROR("Page at target address isn't MEM_COMMIT (0x%016X)", address);
		return nullptr;
	}
		

	DWORD oldProtection = -1;
	if ((information.AllocationProtect & PAGE_READWRITE) == 0 && (information.AllocationProtect & PAGE_EXECUTE_READWRITE) == 0)
	{
		if (VirtualProtect(reinterpret_cast<void*>(address), value.size(), PAGE_EXECUTE_READWRITE, &oldProtection) == FALSE)
		{
			LOG_LAST_ERROR("Failed to change page protection");
			return nullptr;
		}
	}

	auto oldValue = new std::vector<uint8_t>(value.size());
	auto errorCode = memcpy_s(oldValue->data(), value.size(), reinterpret_cast<void*>(address), value.size());
	if (errorCode != 0)
	{
		LOG_ERROR("Failed to get origin value from memory at 0x%016X. Error code: %d", address, errorCode);
		delete oldValue;
		return nullptr;
	}

	errorCode = memcpy_s(reinterpret_cast<void*>(address), value.size(), value.data(), value.size());
	if (errorCode != 0)
	{
		LOG_ERROR("Failed to rewrite target memory at 0x%016X. Error code: %d", address, errorCode);
		delete oldValue;
		return nullptr;
	}

	if (oldProtection != -1)
	{
		DWORD temp = 0;
		if (VirtualProtect(reinterpret_cast<void*>(address), value.size(), oldProtection, &temp) == FALSE)
		{
			LOG_LAST_ERROR("Failed to restore page protection");
		}
	}
	return oldValue;
}
