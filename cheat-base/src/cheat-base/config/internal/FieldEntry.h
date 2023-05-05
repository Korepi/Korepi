#pragma once
#include <string>
#include <cheat-base/events/event.hpp>
#include <nlohmann/json.hpp>

namespace config::internal
{
	class FieldEntry
	{
	public:
		FieldEntry(const std::string& name, const std::string& sectionName, bool multiProfile = false)
			: m_Name(name), m_Section(sectionName), m_MultiProfile(multiProfile), m_Container(nullptr) {}

		TEvent<FieldEntry*> ChangedEvent;
		TEvent<FieldEntry*, const std::string&, bool> MovedEvent;
		TEvent<FieldEntry*, const std::string&, bool> RepositionEvent;

		inline virtual void FireChanged()
		{
			ChangedEvent(this);
		}

		virtual nlohmann::json ToJson() = 0;
		virtual void FromJson(const nlohmann::json& value) = 0;
		virtual void Reset() = 0;

		inline bool IsShared() const
		{
			return m_MultiProfile;
		}

		inline std::string GetName() const
		{
			return m_Name;
		}

		inline std::string GetSection() const
		{
			return m_Section;
		}

		inline nlohmann::json* GetContainer() const
		{
			return m_Container;
		}

		inline void Reposition(const std::string& newSection, bool shared = false)
		{
			std::string oldSection = m_Section;
			bool oldMultiProfile = m_MultiProfile;

			m_Section = newSection;
			m_MultiProfile = shared;

			RepositionEvent(this, newSection, shared);
		}

		inline void Move(const std::string& newSection, bool shared = false)
		{
			std::string oldSection = m_Section;
			bool oldMultiProfile = m_MultiProfile;

			m_Section = newSection;
			m_MultiProfile = shared;

			MovedEvent(this, oldSection, oldMultiProfile);
		}

		inline void SetContainer(nlohmann::json* newContainer)
		{
			m_Container = nullptr;
		}

	protected:
		std::string m_Name;
		std::string m_Section;
		bool m_MultiProfile;

		nlohmann::json* m_Container;
	};
}