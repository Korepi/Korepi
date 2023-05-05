#include <pch-il2cpp.h>

#include "ChestFilter.h"

namespace cheat::game 
{
	ChestFilter::ChestFilter(game::Chest::ChestRarity rarity) : m_ItemType(Chest::ItemType::Chest), m_Rarity(rarity)
	{ }

	ChestFilter::ChestFilter(game::Chest::ItemType itemType) : m_ItemType(itemType)
	{ }

	ChestFilter::ChestFilter(game::Chest::ChestState state) : m_ItemType(Chest::ItemType::Chest), m_State(state)
	{ }

	bool ChestFilter::IsValid(game::Entity* entity) const
	{
		if (!entity->isChest())
			return false;

		game::Chest* chest = dynamic_cast<game::Chest*>(entity);
		
		return 
			(!m_ItemType || chest->itemType() == *m_ItemType) && 
			(!m_Rarity || chest->chestRarity() == *m_Rarity) && 
			(!m_State  || chest->chestState() == *m_State);
	}

	ChestFilter operator+(ChestFilter lFilter, const ChestFilter& rFilter)
	{
		if (rFilter.m_ItemType != Chest::ItemType::Chest)
			return lFilter;
		
		lFilter.m_ItemType = Chest::ItemType::Chest;

		if (rFilter.m_Rarity)
			lFilter.m_Rarity = rFilter.m_Rarity;

		if (rFilter.m_State)
			lFilter.m_State = rFilter.m_State;
		
		return lFilter;
	}

}


