#include "pch-il2cpp.h"
#include "ESPCustomFilter.h"

namespace cheat::feature
{
	bool ESPCustomFilter::IsValid(game::Entity* entity) const
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
