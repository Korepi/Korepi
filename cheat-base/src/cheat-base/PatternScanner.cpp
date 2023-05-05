#include "pch.h"
#include "PatternScanner.h"

#include <psapi.h>

#include <fstream>
#include <functional>
#include <cheat-base/util.h>

PatternScanner::PatternScanner() : 
	m_CacheChanged(false)
{
}

void PatternScanner::ParseSignatureFile(const std::string& signaturesContent)
{
	nlohmann::json siganturesJson;
	try
	{
		siganturesJson = nlohmann::json::parse(signaturesContent);
	}
	catch (nlohmann::json::parse_error* e)
	{
		LOG_ERROR("Failed to parse siganature json content. Error byte %d", e->byte);
		return;
	}

	ParseSignatureJson(&siganturesJson);
}

bool PatternScanner::IsUpdated()
{
	return m_CacheChanged;
}

void PatternScanner::ParseSignatureJson(void* signatureJson)
{
	nlohmann::json& jsonContent = *reinterpret_cast<nlohmann::json*>(signatureJson);
	for (auto& moduleEntry : jsonContent.items())
	{
		std::string moduleName = moduleEntry.key();

		if (m_ModulePatterns.count(moduleName) == 0)
			m_ModulePatterns[moduleName] = {};

		auto& functionPatterns = m_ModulePatterns[moduleName];
		for (auto& functionEntry : moduleEntry.value().items())
		{
			std::string functionName = functionEntry.key();
			functionPatterns[functionName] = {};

			auto& container = functionPatterns[functionName];
			auto& patternInfo = functionEntry.value();

			for (auto& xref : patternInfo["xref"])
				container.xrefs.push_back({ xref["sig"], xref["offset"] });

			for (auto& pattern : patternInfo["signatures"])
				container.signatures.push_back(pattern);
		}
	}
}

uintptr_t PatternScanner::GetOffsetInt(const nlohmann::json& value)
{
	std::uintptr_t offset = 0;
	if (value.is_string())
	{
		std::string strValue = value;
		offset = strtoul(strValue.c_str(), nullptr, 16);
	}
	else if (value.is_number_unsigned())
	{
		offset = value;
	}
	return offset;
}


std::string PatternScanner::GetOffsetStr(uintptr_t offset)
{
	std::stringstream ss;
	ss << std::hex << offset;
	return ss.str();
}

void PatternScanner::Save(const std::filesystem::path& filename)
{
	std::ofstream outputStream(filename, std::ios::out);
	if (!outputStream.is_open())
	{
		LOG_ERROR("Failed to open file '%s' to save offsets.", filename.c_str());
		return;
	}

	std::string output;
	Save(output);
	outputStream << output;
	outputStream.close();
}

void PatternScanner::Save(std::string& outContent)
{
	if (!m_CacheChanged)
		outContent = m_LoadCache;

	nlohmann::json modulesInfo{};
	
	SaveJson(modulesInfo);

	outContent = modulesInfo.dump();
}

void PatternScanner::SaveJson(nlohmann::json& outObject)
{
	for (auto& [moduleName, functionsOffsets] : m_CacheOffsets)
	{
		SaveModuleHash(moduleName, outObject[moduleName]["hash"]);

		auto& functionsObject = outObject[moduleName]["functions"];
		for (auto& [functionName, offset] : functionsOffsets)
		{
			functionsObject[functionName] = GetOffsetStr(offset);
		}
	}
}

bool PatternScanner::Load(const std::filesystem::path& filename)
{
	std::ifstream inputStream(filename, std::ios::in);
	if (!inputStream.is_open())
	{
		LOG_ERROR("Failed to open file '%s' for load offsets.", filename.c_str());
		return false;
	}

	std::stringstream buffer;
	buffer << inputStream.rdbuf();
	return Load(buffer.str());
}

bool PatternScanner::Load(const std::string& content)
{
	nlohmann::json contentJson;
	try
	{
		contentJson = nlohmann::json::parse(content);
	}
	catch (nlohmann::json::parse_error* e)
	{
		LOG_ERROR("Failed to parse siganature json content. Error byte %d", e->byte);
		return false;
	}

	return LoadJson(contentJson);

	m_LoadCache = content;
}

bool PatternScanner::LoadJson(const nlohmann::json& object)
{
	bool result = true;
	for (auto& moduleEntry : object.items())
	{
		std::string moduleName = moduleEntry.key();
		auto& moduleJson = moduleEntry.value();

		if (!IsValidModuleHash(moduleName, moduleJson["hash"]))
		{
			LOG_WARNING("Module '%s' hash don't match with saved one. Seems module was updated.", moduleName.c_str());
			result = false;
			continue;
		}

		if (m_CacheOffsets.count(moduleName) == 0)
			m_CacheOffsets[moduleName] = {};

		auto& functionsOffsets = m_CacheOffsets[moduleName];
		for (auto& funcOffsetEntry : moduleJson["functions"].items())
		{
			functionsOffsets[funcOffsetEntry.key()] = GetOffsetInt(funcOffsetEntry.value());
		}
	}
	return result;
}

PatternScanner::ModuleInfo& PatternScanner::GetModuleInfo(const std::string& modulePath)
{
	static std::map<std::string, ModuleInfo> s_ModuleInfoCache;
	std::string moduleName = std::filesystem::path(modulePath).filename().string();

	if (s_ModuleInfoCache.count(moduleName) > 0)
		return s_ModuleInfoCache[moduleName];

	HMODULE hModule = GetModuleHandle(moduleName.c_str());
	if (hModule == NULL)
	{
		LOG_LAST_ERROR("Failed to find module '%s'.", moduleName.c_str());
		std::system("pause");
		exit(0);
	}
	
	s_ModuleInfoCache[moduleName] = GetModuleInfo(hModule);
	return s_ModuleInfoCache[moduleName];
}

PatternScanner::ModuleInfo& PatternScanner::GetModuleInfo(HMODULE hModule)
{
	static std::map<HMODULE, ModuleInfo> s_ModuleInfoCache;

	if (hModule == NULL)
	{
		LOG_CRIT("hModule is NULL.");
		std::system("pause");
		exit(0);
	}

	if (s_ModuleInfoCache.count(hModule) > 0)
		return s_ModuleInfoCache[hModule];

	MODULEINFO nativeInfo{};
	BOOL result = GetModuleInformation(GetCurrentProcess(), hModule, &nativeInfo, sizeof(nativeInfo));
	if (result == FALSE)
	{
		LOG_LAST_ERROR("Failed get info about module at 0x%p.", hModule);
		std::system("pause");
		exit(0);
	}

	s_ModuleInfoCache[hModule] = {};
	auto& moduleInfo = s_ModuleInfoCache[hModule];
	moduleInfo.handle = hModule;
	moduleInfo.base = (uintptr_t)hModule;
	moduleInfo.size = (uintptr_t)nativeInfo.SizeOfImage;

	char buffer[MAX_PATH] = {};
	result = GetModuleFileNameA(hModule, buffer, sizeof(buffer));
	if (result == FALSE)
	{
		LOG_LAST_ERROR("Failed get filename module at 0x%p.", hModule);
		std::system("pause");
		exit(0);
	}

	moduleInfo.filePath = buffer;

	LOG_DEBUG("Module %s bound 0x%p-0x%p.", moduleInfo.filePath.c_str(), 
		moduleInfo.base, moduleInfo.base + moduleInfo.size);

	uintptr_t currentAddress = moduleInfo.base;
	uintptr_t endAddress = moduleInfo.base + moduleInfo.size;
	while (currentAddress <= endAddress)
	{
		MEMORY_BASIC_INFORMATION memoryInfo{};
		auto byteCount = VirtualQuery((LPCVOID)currentAddress, &memoryInfo, sizeof(memoryInfo));
		if (byteCount == 0)
		{
			LOG_LAST_ERROR("Failed get memory info for address 0x%p.", currentAddress);
			break;
		}

		currentAddress = (uintptr_t)memoryInfo.BaseAddress + memoryInfo.RegionSize;

		auto protection = memoryInfo.AllocationProtect;

		if (protection != PAGE_EXECUTE && protection != PAGE_EXECUTE_READ &&
			protection != PAGE_EXECUTE_READWRITE && protection != PAGE_EXECUTE_WRITECOPY)
			continue;

		moduleInfo.execRegions.push_back({ (uintptr_t)memoryInfo.BaseAddress, memoryInfo.RegionSize });
	}

	return moduleInfo;
}

void PatternScanner::AddOffset(const std::string& moduleName, const std::string& name, uintptr_t offset)
{
	if (m_CacheOffsets.count(moduleName) == 0)
		m_CacheOffsets[moduleName] = {};

	m_CacheOffsets[moduleName][name] = offset;
	m_CacheChanged = true;
}

size_t ComputeChecksum(const std::string& filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		LOG_ERROR("Failed to compute file json: %s", filename.c_str());
		return 0;
	}

	size_t sum = 0;
	size_t word = 0;
	while (file.read(reinterpret_cast<char*>(&word), sizeof(word))) {
		sum += word;
	}

	if (file.gcount()) {
		word &= (~0U >> ((sizeof(size_t) - file.gcount()) * 8));
		sum += word;
	}

	return sum;
}

int64_t PatternScanner::GetModuleTimestamp(const std::string& moduleName)
{
	auto& moduleInfo = GetModuleInfo(moduleName);
	auto write_time = std::filesystem::last_write_time(moduleInfo.filePath);
	return write_time.time_since_epoch().count();
}

int64_t PatternScanner::GetModuleTimestamp(HMODULE hModule)
{
	auto& moduleInfo = GetModuleInfo(hModule);
	auto write_time = std::filesystem::last_write_time(moduleInfo.filePath);
	return write_time.time_since_epoch().count();
}

bool PatternScanner::IsValidModuleHash(const std::string& moduleName, const nlohmann::json& hashObject)
{
	auto& info = GetModuleInfo(moduleName);
	return IsValidModuleHash(info.handle, hashObject);
}

bool PatternScanner::IsValidModuleHash(HMODULE hModule, const nlohmann::json& hashObject)
{
	if (!hashObject.contains("timestamp") || !hashObject.contains("checksum"))
		return false;

	int64_t currTimestamp = GetModuleTimestamp(hModule);

	int64_t timestamp = hashObject["timestamp"];
	size_t checksum = hashObject["checksum"];

	// To increase speed, we don't check checksum if timestamp matches
	if (timestamp == currTimestamp)
	{
		m_ComputedHashes[hModule] = checksum;
		return true;
	}

	size_t currChecksum = m_ComputedHashes.count(hModule) > 0 ? m_ComputedHashes[hModule] : ComputeChecksum(GetModuleInfo(hModule).filePath);
	m_ComputedHashes[hModule] = currChecksum;

	return checksum == currChecksum;
}

void PatternScanner::SaveModuleHash(const std::string& moduleName, nlohmann::json& outObject)
{
	auto& info = GetModuleInfo(moduleName);
	SaveModuleHash(info.handle, outObject);
}

void PatternScanner::SaveModuleHash(HMODULE hModule, nlohmann::json& outObject)
{
	auto& moduleInfo = GetModuleInfo(hModule);
	auto write_time = std::filesystem::last_write_time(moduleInfo.filePath);
	int64_t timestamp = write_time.time_since_epoch().count();

	size_t checksum = m_ComputedHashes.count(hModule) > 0 ? m_ComputedHashes[hModule] : ComputeChecksum(moduleInfo.filePath);

	outObject["timestamp"] = timestamp;
	outObject["checksum"] = checksum;

	m_ComputedHashes[hModule] = checksum;
}

void PatternScanner::SearchAll()
{
	for (auto& [moduleName, methodsSignatures] : m_ModulePatterns)
	{
		for (auto& [methodName, sigInfo] : methodsSignatures)
		{
			LOG_DEBUG("Searching %s::%s", moduleName.c_str(), methodName.c_str());
			auto searchResult = Search(moduleName, methodName);
			if (searchResult == 0)
				LOG_WARNING("Not found");
			else
				LOG_DEBUG("Found function at %s + %p", moduleName.c_str(), searchResult);
		}
	}
}

uintptr_t PatternScanner::Search(const std::string& name)
{	
	for (auto& [moduleName, modulePatternsData] : m_ModulePatterns)
	{
		if (modulePatternsData.count(name) > 0)
			return Search(moduleName, name, modulePatternsData[name]);
	}

	return 0;
}

uintptr_t PatternScanner::Search(const std::string& moduleName, const std::string& name)
{
	uintptr_t moduleBase = GetModuleInfo(moduleName).base;

	if (m_CacheOffsets.count(moduleName) > 0 && m_CacheOffsets[moduleName].count(name) > 0)
	{
		uintptr_t offset = m_CacheOffsets[moduleName][name];
		return offset != 0 ? moduleBase + offset : 0;
	}

	if (m_ModulePatterns.count(moduleName) > 0 && m_ModulePatterns[moduleName].count(name) > 0)
		return Search(moduleName, name, m_ModulePatterns[moduleName][name]);

	AddOffset(moduleName, name, 0);
	return 0;
}

uintptr_t PatternScanner::Search(const std::string& moduleName, const std::string& name, const PatternInfo& info)
{
	AddressCounter counter;
	uintptr_t moduleBase = GetModuleInfo(moduleName).base;

	for (auto& pattern : info.signatures)
	{
		auto address = SearchFunction(moduleName, pattern);
		if (address == 0)
			continue;

		counter.Add(address);
	}

	for (auto& xrefPattern : info.xrefs)
	{
		auto address = SearchXref(moduleName, xrefPattern);
		if (address == 0)
			continue;

		counter.Add(address);
	}

	uintptr_t address = counter.GetMax();
	AddOffset(moduleName, name, address == 0 ? 0 : address - moduleBase);
	return address;
}

uintptr_t PatternScanner::SearchFunction(const std::string& moduleName, const std::string& pattern)
{
	auto address = SearchInModule(moduleName, pattern);
	if (address == 0)
		return 0;

	return FindFunctionEntry(address);
}

uintptr_t PatternScanner::SearchXref(const std::string& moduleName, const OffsetSignature& xrefPattern)
{
	HMODULE hModule = GetModuleInfo(moduleName).handle;
	return SearchXref(hModule, xrefPattern);
}

uintptr_t PatternScanner::SearchXref(HMODULE hModule, const OffsetSignature& xrefPattern)
{
	auto address = SearchInModule(hModule, xrefPattern.pattern);
	if (!address)
		return 0;

	uint8_t callOpcode = util::ReadMapped<uint8_t>((void*)address, xrefPattern.offset, true);
	int opcodeOffset = 0;
	switch (callOpcode)
	{
	case 0xE8:
	case 0xE9:
		opcodeOffset = 1;
		break;
	case 0x48:
	case 0x4C:
		opcodeOffset = 3;
		break;
	default:
		LOG_WARNING("Trying find xref to not supported long call (opcode 0x%x)", callOpcode);
		return 0;
	}

	int callOffset = util::ReadMapped<int>((void*)address, xrefPattern.offset + opcodeOffset, true);
	uintptr_t dataAddress = address + xrefPattern.offset + 4 + opcodeOffset + callOffset;

	//if (!IsFunctionEntry(functionAddress))
	//{
	//	LOG_WARNING("Xref calc function address failed. There is no function at 0x%p.", functionAddress);
	//	return {};
	//}

	return dataAddress;
}

uintptr_t PatternScanner::SearchInModule(const std::string& moduleName, const std::string& pattern)
{
	auto& moduleInfo = GetModuleInfo(moduleName);
	
	auto tokens = util::StringSplit(" ", pattern);

	std::vector<std::optional<uint8_t>> bytePattern;
	bytePattern.reserve(tokens.size());
	for (auto& token : tokens)
	{
		std::optional<uint8_t> value = token == "??" ? std::optional<uint8_t>() : std::stoi(token, 0, 16);
		bytePattern.push_back(value);
	}

	uint8_t countFound = 0;
	uint64_t address = 0;
	for (auto& region : moduleInfo.execRegions)
	{
		auto regionSearchResult = SearchInRange(region.base, region.base + region.size, bytePattern);
		if (regionSearchResult.status == SRStatus::NotFound)
			continue;
		
		if (regionSearchResult.status == SRStatus::NotUnique)
		{
			LOG_WARNING("Pattern ununique '%s'.", pattern.c_str());
			return {};
		}

		countFound++;
		if (countFound > 1)
		{
			LOG_WARNING("Pattern ununique '%s'.", pattern.c_str());
			return {};
		}
			
		address = regionSearchResult.value;
	}

	if (countFound == 0)
		LOG_WARNING("Pattern not found '%s'.", pattern.c_str());
	
	return address;
}

uintptr_t PatternScanner::SearchInModule(HMODULE hModule, const std::string& pattern)
{
	char buffer[MAX_PATH] = {};
	auto count = GetModuleFileNameA(hModule, buffer, sizeof(buffer));
	if (count == 0)
	{
		LOG_ERROR("Failed to get module name for handle 0x%p.", hModule);
		return {};
	}
	return SearchInModule(buffer, pattern);
}

UINT32 get_first_bit_set(UINT32 x)
{
	// Generates a single BSF instruction
	unsigned long ret;
	_BitScanForward(&ret, x);
	return (UINT32)ret;
}

UINT32 clear_leftmost_set(UINT32 value)
{
	// Generates a single BLSR instruction
	return value & (value - 1);
}

int memcmp_mask(const BYTE* buffer1, const BYTE* buffer2, const BYTE* mask2, size_t count)
{
	while (count--)
	{
		if (*mask2)
		{
			if (*buffer1 != *buffer2)
				return -1;
		}

		buffer1++, buffer2++, mask2++;
	};
	return 0;
}

struct Signature
{
	std::vector<uint8_t> bytes;
	std::vector<uint8_t> mask;
	bool hasWildcards = false;

	Signature(const std::vector<std::optional<uint8_t>>& sig)
	{
		for (auto& item : sig)
		{
			if (!item)
			{
				bytes.push_back(0xCC);
				mask.push_back(0x00);
				hasWildcards = true;
				continue;
			}

			bytes.push_back(*item);
			mask.push_back(0xFF);
		}
	}
};

// Find signiture pattern in memory
PBYTE FindSignatureAVX2(PBYTE data, size_t size, const Signature& sig)
{
	const auto* pat = sig.bytes.data();
	size_t patLen = sig.bytes.size();
	size_t patLen1 = (patLen - 1);
	size_t patLen2 = (patLen - 2);

	// Fill 'first' and 'last' with the first and last pattern byte respectively
	const __m256i first = _mm256_set1_epi8(pat[0]);
	const __m256i last = _mm256_set1_epi8(pat[patLen1]);

	if (!sig.hasWildcards)
	{
		// A little faster without wildcards

		// Scan 32 bytes at the time..
		for (size_t i = 0; i + 32 + patLen1 < size; i += 32)
		{
			// Load in the next 32 bytes of input first and last
			// Can use align 32 bit read for first since the input is page aligned
			const __m256i block_first = _mm256_load_si256((const __m256i*) (data + i));
			const __m256i block_last = _mm256_loadu_si256((const __m256i*) (data + i + patLen1));

			// Compare first and last data to get 32byte masks
			const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
			const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);

			// AND the equality masks and into a 32 bit mask
			UINT32 mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));

			// Do pattern compare between first and last position if we got our first and last at this data position
			while (mask != 0)
			{
				UINT32 bitpos = get_first_bit_set(mask);
				if (memcmp(data + i + bitpos + 1, pat + 1, patLen2) == 0)
					return data + i + bitpos;

				mask = clear_leftmost_set(mask);
			};
		}

	}
	else
	{
		// Pattern scan with wildcards mask
		const BYTE* msk = sig.mask.data();

		for (size_t i = 0; i + patLen1 + 32 < size; i += 32)
		{
			const __m256i block_first = _mm256_load_si256((const __m256i*) (data + i));
			const __m256i block_last = _mm256_loadu_si256((const __m256i*) (data + i + patLen1));

			const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
			const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);

			UINT32 mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));

			// Do a byte pattern w/mask compare between first and last position if we got our first and last
			while (mask != 0)
			{
				UINT32 bitpos = get_first_bit_set(mask);
				if (memcmp_mask(data + i + bitpos + 1, pat + 1, msk + 1, patLen2) == 0)
					return data + i + bitpos;

				mask = clear_leftmost_set(mask);
			};
		}
	}
	return NULL;
}

PBYTE FindSignature(PBYTE input, size_t inputLen, const Signature& sig)
{
	if (!sig.hasWildcards)
	{
		// If no wildcards, faster to use a memcmp() type
		const BYTE* pat = sig.bytes.data();
		const BYTE* end = (input + inputLen);
		const BYTE first = *pat;
		size_t sigLen = sig.bytes.size();

		// Setup last in the pattern length byte quick for rejection test
		size_t lastIdx = (sigLen - 1);
		BYTE last = pat[lastIdx];

		for (PBYTE ptr = input; ptr < end; ++ptr)
		{
			if ((ptr[0] == first) && (ptr[lastIdx] == last))
			{
				if (memcmp(ptr + 1, pat + 1, sigLen - 2) == 0)
					return ptr;
			}
		}
	}
	else
	{
		const BYTE* pat = sig.bytes.data();
		const BYTE* msk = sig.mask.data();
		const BYTE* end = (input + inputLen);
		const BYTE first = *pat;
		size_t sigLen = sig.bytes.size();
		size_t lastIdx = (sigLen - 1);
		BYTE last = pat[lastIdx];

		for (PBYTE ptr = input; ptr < end; ++ptr)
		{
			if ((ptr[0] == first) && (ptr[lastIdx] == last))
			{
				const BYTE* patPtr = pat + 1;
				const BYTE* mskPtr = msk + 1;
				const BYTE* memPtr = ptr + 1;
				BOOL found = TRUE;

				for (int i = 0; (i < sigLen - 2) && (memPtr < end); ++mskPtr, ++patPtr, ++memPtr, i++)
				{
					if (!*mskPtr)
						continue;

					if (*memPtr != *patPtr)
					{
						found = FALSE;
						break;
					}
				}

				if (found)
					return ptr;
			}
		}
	}

	return 0;
}

PatternScanner::SearchResult PatternScanner::SearchSignatureAVX2(PBYTE input, size_t inputLen, const std::vector<std::optional<uint8_t>>& pattern)
{
	Signature sig = Signature(pattern);

	size_t sigSize = sig.bytes.size();
	size_t len = inputLen;
	size_t count = 0;

	inputLen -= sigSize;

	PBYTE match = FindSignatureAVX2(input, len, sig);
	uintptr_t firstMatch = (uintptr_t)match;
	while (match)
	{
		if (++count >= 2)
			break;

		++match;
		len = (inputLen - (int)(match - input));
		if (len < sigSize)
			break;

		match = FindSignatureAVX2(match, len, sig);
	};

	SearchResult result {};
	result.value = (uintptr_t)firstMatch;
	switch (count)
	{
	case 0: result.status = SRStatus::NotFound; break;
	case 1: result.status = SRStatus::Unique; break;
	default: result.status = SRStatus::NotUnique; break;
	};
	return result;
}

PatternScanner::SearchResult PatternScanner::SearchSignature(PBYTE input, size_t inputLen, const std::vector<std::optional<uint8_t>>& pattern)
{
	Signature sig = Signature(pattern);

	size_t sigSize = sig.bytes.size();
	size_t len = inputLen;
	size_t count = 0;

	inputLen -= sigSize;

	// Search for signature match..
	PBYTE match = FindSignature(input, len, sig);
	uintptr_t firstMatch = (uintptr_t)match;
	while (match)
	{
		// Stop now if we've hit two matches
		if (++count >= 2)
			break;

		++match;
		len = (inputLen - (int)(match - input));
		if (len < sigSize)
			break;

		// Next search
		match = FindSignature(match, len, sig);
	};

	SearchResult result{};
	result.value = (uintptr_t)firstMatch;
	switch (count)
	{
	case 0: result.status = SRStatus::NotFound; break;
	case 1: result.status = SRStatus::Unique; break;
	default: result.status = SRStatus::NotUnique; break;
	};
	return result;
}

bool TestAVX2Support()
{
	enum { EAX, EBX, ECX, EDX };
	int regs[4];

	// Highest Function Parameter
	__cpuid(regs, 0);
	if (regs[EAX] >= 7)
	{
		// Extended Features
		__cpuid(regs, 7);
		return (regs[EBX] & /*AVX2*/ (1 << 5)) != 0;
	}
	return false;
}

PatternScanner::SearchResult PatternScanner::SearchInRange(uintptr_t start, uintptr_t end, const std::vector<std::optional<uint8_t>>& pattern)
{
	bool hasAVX2 = TestAVX2Support();
	
	if (hasAVX2)
	{
		auto result = SearchSignatureAVX2((PBYTE)start, end - start, pattern);
		return result;
	}
	
	static bool warnOnce = true;
	if (warnOnce)
	{
		warnOnce = false;
		LOG_WARNING("Using non-AVX2 reference search *\n");
	}

	return SearchSignature((PBYTE)start, end - start, pattern);


	/*const uint8_t* rStart = (const uint8_t*)start;
	const uint8_t* rEnd = (const uint8_t*)end;

	auto comparer = [](uint8_t val1, std::optional<uint8_t> val2)
	{
		return (!val2 || val1 == *val2);
	};

	SearchResult sResult = { SRStatus::NotFound, 0 };
	while (true)
	{
		const uint8_t* res = std::search(rStart, rEnd, pattern.begin(), pattern.end(), comparer);
		if (res >= rEnd)
			break;

		if (sResult.status != SRStatus::NotFound)
			return { SRStatus::NotUnique, 0 };

		sResult = { SRStatus::Unique, (uint64_t)res };
		rStart = res + pattern.size();
	}
	return sResult;*/
}

bool PatternScanner::IsFunctionEntry(uintptr_t functionAddress)
{
	auto address = FindFunctionEntry(functionAddress);
	if (functionAddress == 0)
		return false;
	return functionAddress == functionAddress;
}

uintptr_t PatternScanner::FindFunctionEntry(uintptr_t address)
{
	// TODO: Implement function asm head find. Maybe with use Zydis.
	return 0;
}

PatternScanner::AddressCounter::AddressCounter() {}

void PatternScanner::AddressCounter::Add(uintptr_t address)
{
	if (m_Counts.count(address) == 0)
		m_Counts[address] = 0;
	m_Counts[address]++;
}

uintptr_t PatternScanner::AddressCounter::GetMax()
{
	if (m_Counts.size() == 0)
		return 0;

	using pair_type = decltype(m_Counts)::value_type;
	auto maxEntry = std::max_element(m_Counts.begin(), m_Counts.end(),
		[](const pair_type& a, const pair_type& b)
		{
			return a.second < b.second;
		});

	return maxEntry->first;
}
