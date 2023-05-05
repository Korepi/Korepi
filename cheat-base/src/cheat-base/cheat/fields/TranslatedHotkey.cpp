#include <pch.h>
#include "TranslatedHotkey.h"

#include <cheat-base/Translator.h>

namespace cheat
{
	TranslatedHotkey::TranslatedHotkey(const std::string& name, bool enabled)
		: NamedHotkey(name, enabled) {}

	TranslatedHotkey::TranslatedHotkey(const std::string& name, const Hotkey& hotkey)
		: NamedHotkey(name, hotkey) {}

	std::string TranslatedHotkey::name()
	{
		return Translator::RuntimeTranslate(NamedHotkey::name());
	}
}
