#pragma once

#include <imgui.h>

#include <cheat-base/config/config.h>

namespace cheat::feature::esp
{
	class ESPItem
	{
	public:
		ESPItem(const std::string& name, const ImColor& color, const Hotkey& hotkey, const std::string& textureName)
			: m_Name(name), m_Color(color), m_Hotkey(hotkey), m_Texture(nullptr), m_Enabled(false) { }

		bool operator==(const ESPItem& another) const
		{
			return m_Enabled == another.m_Enabled && m_Color == another.m_Color && m_Hotkey == another.m_Hotkey;
		}

		Hotkey m_Hotkey;
		bool m_Enabled;
		std::string m_Name;
		ImColor m_Color;
		ImColor m_ContrastColor;
		void* m_Texture;
	};
}

namespace config::converters
{
	template<>
	inline nlohmann::json ToJson(const cheat::feature::esp::ESPItem& value)
	{
		return {
			{ "enabled", value.m_Enabled },
			{ "color", ToJson(value.m_Color) },
			{ "hotkey", ToJson(value.m_Hotkey) }
		};
	}

	template<>
	inline void FromJson(cheat::feature::esp::ESPItem& value, const nlohmann::json& jObject)
	{
		value.m_Enabled = jObject["enabled"];
		FromJson(value.m_Hotkey, jObject["hotkey"]);
		FromJson(value.m_Color, jObject["color"]);
	}
}