#pragma once
#include <cheat-base/config/fields/Toggle.h>
#include <cheat-base/Hotkey.h>

namespace cheat
{
	class NamedHotkey : public config::Toggle<Hotkey>
	{
	public:
		NamedHotkey(const std::string& name, bool enabled);
		NamedHotkey(const std::string& name, const Hotkey& hotkey);

		virtual std::string name();

	private:
		std::string m_Name;
	};
}