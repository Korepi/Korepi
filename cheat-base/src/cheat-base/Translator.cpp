#include "pch.h"
#include "Translator.h"

void Translator::Init(const std::filesystem::path& templatePath)
{
	std::ifstream ifs(templatePath.wstring(), std::ios::binary | std::ios::in);
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	Init(content);
}

void Translator::Init(const std::string& content)
{
	Init(nlohmann::json::parse(content));
}

void Translator::Init(const nlohmann::json& json)
{
	_languages.clear();
	_languagesInfo = json;

	for (auto& [key, value] : _languagesInfo.items())
		_languages.push_back(key);

	_languages.emplace_back("default");
}

std::string Translator::RuntimeTranslate(const std::string& origin)
{
	return FindTranslationString(origin);
}

void Translator::SetDefaultPseudo(const std::string& pseudo)
{
	_defaultLanguagePseudo = pseudo;
}

void Translator::SetDefaultFontName(const std::string& fontName)
{
	_defaultFontName = fontName;
}

const std::string& Translator::GetCurrentFontName()
{
	return _currentFont;
}

const std::string& Translator::GetCurrentLanguage()
{
	return _currentLanguage;
}

const std::vector<std::string>& Translator::GetLanguages()
{
	return _languages;
}

const std::string Translator::GetLanguagePseudo(const std::string& language)
{
	if (language == "default")
		return _defaultLanguagePseudo;

	if (!_languagesInfo.contains(language))
		return "";

	return _languagesInfo[language]["pseudo"];
}

void Translator::SetLanguage(const std::string& language)
{
	if (language == _currentLanguage)
		return;

	if (language == "default")
	{
		_translations.clear();
		_currentLanguage = language;
		_currentFont = _defaultFontName;
		NotifyUpdate();
		return;
	}

	if (!_languagesInfo.contains(language))
		return;

	_translations.clear();

	auto& translates = _languagesInfo[language]["translates"];
	for (auto& [key, value] : translates.items())
		_translations[key] = value;

	_currentLanguage = language;
	_currentFont = _languagesInfo[language]["font"];
	NotifyUpdate();
}

std::string Translator::FindTranslationString(const std::string& origin)
{
#ifdef _DEBUG
	if (!_debugOrigins.contains(origin))
		_debugOrigins.insert(origin);
#endif

	if (_currentLanguage == "default")
		return origin;

	auto it = _translations.find(origin);
	if (it == _translations.end())
		return origin;

	return it->second;
}

void Translator::NotifyUpdate()
{
	for (auto& updateFlag : _updateFlags)
		updateFlag = true;

	_languageChangedEvent();
}

#ifdef _DEBUG
nlohmann::json Translator::DumpTranslateConfigTemplate()
{
	nlohmann::json languages = _languagesInfo;

	for (auto& [name, info] : languages.items())
	{
		nlohmann::json tempInfo = info;

		auto translates = info["translates"];
		auto& translatesOrigin = info["translates"];
		for (auto& [origin, translated] : translates.items())
		{
			if (_debugOrigins.contains(origin))
				continue;

			translatesOrigin.erase(origin);
			translatesOrigin[fmt::format("!DEL {}", origin)] = translated;
		}

		for (auto& origin : _debugOrigins)
		{
			if (!translatesOrigin.contains(origin))
				translatesOrigin[origin] = origin;
		}
	}
	return languages;
}
#endif
