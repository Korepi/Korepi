#include <pch.h>
#include "NamedHotkey.h"

namespace cheat
{
	NamedHotkey::NamedHotkey(const std::string& name, bool enabled)
		: m_Name(name), config::Toggle<Hotkey>(enabled)
	{ }

	NamedHotkey::NamedHotkey(const std::string& name, const Hotkey& hotkey)
		: m_Name(name), config::Toggle<Hotkey>(hotkey)
	{ }

	std::string NamedHotkey::name()
	{
		return m_Name;
	}
}