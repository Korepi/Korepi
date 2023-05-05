#pragma once

#include <optional>

#include "IEntityFilter.h"
#include "Chest.h"

namespace cheat::game
{
	class ChestFilter : public IEntityFilter
	{
	public:
		ChestFilter(game::Chest::ChestRarity rarity);
		ChestFilter(game::Chest::ChestState state);
		ChestFilter(game::Chest::ItemType itemType);

		bool IsValid(game::Entity* entity) const override;

		friend ChestFilter operator+(ChestFilter lFilter, const ChestFilter& rFilter);
	
	private:

		std::optional<game::Chest::ItemType> m_ItemType;
		std::optional<game::Chest::ChestRarity> m_Rarity;
		std::optional<game::Chest::ChestState> m_State;

	};
}
