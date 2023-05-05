#include "pch-il2cpp.h"
#include "SimpleFilter.h"

namespace cheat::game
{
	SimpleFilter::SimpleFilter(std::initializer_list<SimpleFilter> names)
		: m_Type(names.begin()->m_Type)
	{
		std::for_each(names.begin(), names.end(), [this](const SimpleFilter& other) {
			m_Names.insert(m_Names.begin(), other.m_Names.begin(), other.m_Names.end());
		});
	}

	bool SimpleFilter::IsValid(Entity* entity) const
	{
		if (entity == nullptr)
			return false;

		if (entity->type() != m_Type)
			return false;

		if (m_Names.size() == 0)
			return true;

		auto& name = entity->name();
		for (auto& pattern : m_Names)
		{
			if (name.find(pattern) != -1)
				return true;
		}

		return false;
	}
}
