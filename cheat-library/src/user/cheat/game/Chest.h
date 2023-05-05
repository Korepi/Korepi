#pragma once

#include "Entity.h"

namespace cheat::game
{
	class Chest : public Entity
	{

	public:

		enum class ItemType
		{
			None,
			Chest,
			CrudeChest,
			Investigate,
			BookPage,
			BGM,
			Flora,
			QuestInteract
		};

		enum class ChestState
		{
			Invalid,
			None,
			Locked,
			InRock,
			Frozen,
			Bramble,
			Trap
		};

		enum class ChestRarity
		{
			Unknown,
			Common,
			Exquisite,
			Precious,
			Luxurious,
			Remarkable
		};

		Chest(app::BaseEntity* rawEntity);

		ItemType itemType();
		ChestState chestState();
		ChestRarity chestRarity();
		ImColor chestColor();
		std::string minName();
	private:

		std::optional<ItemType> m_ItemType;
		std::optional<ChestState> m_ChestState;
		std::optional<ChestRarity> m_ChestRarity;
	};
}

