#pragma once

#include <cheat-base/Hotkey.h>
#include <cheat-base/config/Field.h>
#include <cheat-base/ISerializable.h>

namespace config
{
	template<typename T>
	class Toggle : public ISerializable
	{
	public:

		bool enabled() const
		{
			return m_Enabled;
		}

		const T& value() const
		{
			return m_Value;
		}

		T& value()
		{
			return m_Value;
		}

		void set_enabled(bool enable, bool notify = true)
		{
			m_Enabled = enable;
			
			if (notify)
				OnEnableChange(this);
		}

		void set_value(const T& val)
		{
			m_Value = val;
		}

		void to_json(nlohmann::json& j) const override
		{
			j = {
				{ "toggled", m_Enabled },
				{ "value", config::converters::ToJson(m_Value) }
			};
		}

		void from_json(const nlohmann::json& j) override
		{
			if (j.is_boolean())
			{
				m_Enabled = j;
				m_Value = {};
				return;
			}

			m_Enabled = j["toggled"].get<uint32_t>();
			config::converters::FromJson(m_Value, j["value"]);
		}

		explicit Toggle(const T& val) : m_Enabled(false), m_Value(val), OnEnableChange() { }

		explicit Toggle(bool enable) : m_Enabled(enable), m_Value(), OnEnableChange() { }

		explicit Toggle() : m_Enabled(false), m_Value(), OnEnableChange() { }

		inline operator bool()
		{
			return m_Enabled;
		}

		inline bool operator==(const Toggle<T>& rhs)
		{
			return rhs.m_Enabled == m_Enabled && rhs.m_Value == m_Value;
		}

		TEvent<Toggle<T>*> OnEnableChange;

	private:
		bool m_Enabled;
		T m_Value;
	};

	// Okay, close your eyes and don't look at this mess. (Please)
	template<typename T>
	class Field<Toggle<T>> : public internal::FieldBase<Toggle<T>>
	{
	public:
		using base = internal::FieldBase<Toggle<T>>;
		using base::operator=;
		using base::base;

		operator bool() const
		{
			return base::value().enabled();
		}

		operator T&() const
		{
			return base::value().value();
		}
	};
}
