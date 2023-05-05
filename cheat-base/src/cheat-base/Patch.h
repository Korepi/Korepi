#pragma once

#include <vector>
#include <map>

#define OPatch(offset, value) Patch::Install(il2cppi_get_base_address() + offset, value)
#define OUnpatch(offset) Patch::Restore(il2cppi_get_base_address() + offset)
#define TogglePatch(field, targetField, offset, patchBytes) if (field == &targetField) { if (targetField.GetValue()) OPatch(offset, patchBytes); else OUnpatch(offset); return; }

class Patch
{
public:

	// Installing patch to target address.
	// In detail: replaces memory in target address with specified in 'value'
	//            saves old memory for future restore
	// Return true if successfull.
	static bool Install(uint64_t address, std::vector<uint8_t> value);

	// Restoring old memory in this address, if it was patched.
	// Return true if successfull.
	static bool Restore(uint64_t address);

private:

	inline static std::map<uint64_t, std::vector<uint8_t>*> patches{};

	static std::vector<uint8_t>* WriteMemory(uint64_t address, std::vector<uint8_t> value);

};

