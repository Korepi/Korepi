#pragma once
#include <cheat-base/cheat/fields/NamedHotkey.h>
#include <cheat-base/config/Field.h>

namespace cheat
{
	class TranslatedHotkey : public NamedHotkey
	{
	public:
		TranslatedHotkey(const std::string& name, bool enabled);
		TranslatedHotkey(const std::string& name, const Hotkey& hotkey);

		std::string name() final;
	};
}

namespace config {
	template<>
	class Field<cheat::TranslatedHotkey> : public internal::FieldBase<cheat::TranslatedHotkey>
	{
	public:
		using base = internal::FieldBase<cheat::TranslatedHotkey>;
		using base::operator=;
		using base::base;

		operator bool() const
		{
			return base::value().enabled();
		}
	};
}