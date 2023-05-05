#pragma once

#include <magic_enum.hpp>

namespace config
{

	template<typename T>
	class Enum
	{
	public:
		Enum()
		{
			static_assert(std::is_enum<T>::value, "Must be an enum type");
			m_Value = T();
		}

		Enum(T enumValue)
		{
			static_assert(std::is_enum<T>::value, "Must be an enum type");
			m_Value = enumValue;
		}

		inline T value() const
		{
			return m_Value;
		}

		inline T* pointer() const
		{
			return const_cast<T*>(&m_Value);
		}

		inline operator T()
		{
			return value();
		}

		inline T* operator&()
		{
			return pointer();
		}

		inline uint32_t raw() const
		{
			return static_cast<uint32_t>(m_Value);
		}

		inline Enum& operator=(const T& other)
		{
			static_assert(std::is_enum<T>::value, "Must be an enum type");
			m_Value = other;
			return *this;
		}

		inline Enum& operator=(const uint32_t& other)
		{
			m_Value = static_cast<T>(other);
			return *this;
		}

	private:
		T m_Value;
	};


	//// Okay, close your eyes and don't look at this mess. (Please)
	//template <typename K>
	//class Field<Enum<K>> : public internal::FieldBase<Enum<K>>
	//{
	//public:
	//	using base = internal::FieldBase<Enum<K>>;
	//	using base::operator=;
	//	using base::base;

	//	operator T() const
	//	{
	//		return base::value();
	//	}
	//};
}

namespace nlohmann
{
	template <typename T>
	struct adl_serializer<config::Enum<T>> {
		static void to_json(json& j, const config::Enum<T>& enumValue)
		{
			j = {
				{ "name", magic_enum::enum_name(enumValue.value()) },
				{ "value", enumValue.raw() }
			};
		}

		static void from_json(const json& j, config::Enum<T>& value)
		{
			value = j["value"].get<uint32_t>();
		}
	};
}