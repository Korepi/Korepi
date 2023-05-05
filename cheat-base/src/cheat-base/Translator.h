#pragma once
#include <cstdint>
#include <atomic>
#include <type_traits>

#include <nlohmann/json.hpp>
#include <cheat-base/util.h>

#define _TR(ORIGIN) Translator::Translate<COMPILE_TIME_CRC32_STR(ORIGIN)>(ORIGIN)

class Translator
{
public:
	static void Init(const std::filesystem::path& templatePath);
	static void Init(const std::string& content);
	static void Init(const nlohmann::json& json);

	template<uint32_t crcHash>
	static const char* Translate(const std::string& origin)
	{
		static std::atomic<bool>* updateFlag = &_updateFlags.emplace_back(true);
		static std::string translated = origin;
		if (*updateFlag)
		{
			translated = FindTranslationString(origin);
			*updateFlag = false;
		}
		return translated.c_str();
	}

	static std::string RuntimeTranslate(const std::string& origin);

	static void SetDefaultPseudo(const std::string& pseudo);
	static void SetDefaultFontName(const std::string& fontName);

	static const std::string& GetCurrentFontName();
	static const std::string& GetCurrentLanguage();
	static const std::vector<std::string>& GetLanguages();
	static const std::string GetLanguagePseudo(const std::string& translationName);
	static void SetLanguage(const std::string& language);

#ifdef _DEBUG
	static nlohmann::json DumpTranslateConfigTemplate();
#endif

private:
	static std::string FindTranslationString(const std::string& origin);
	static void NotifyUpdate();

	inline static std::list<std::atomic<bool>> _updateFlags;
	inline static std::unordered_map<std::string, std::string> _translations;
	
	inline static std::vector<std::string> _languages;
	inline static std::string _currentLanguage = "default";
	inline static std::string _defaultLanguagePseudo = "English | EN";
	inline static std::string _defaultFontName = "DefaultFont";

	inline static std::string _currentFont = _defaultFontName;
	inline static nlohmann::json _languagesInfo;

#ifdef _DEBUG
	inline static std::unordered_set<std::string> _debugOrigins;
#endif

	static inline TEvent<> _languageChangedEvent;

public:
	static inline IEvent<>& LanguageChangedEvent = _languageChangedEvent;
};

