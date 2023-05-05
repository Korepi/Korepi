#pragma once
#include "ItemTeleportBase.h"

#include <cheat/game/Chest.h>
namespace cheat::feature 
{

	class ChestTeleport : public ItemTeleportBase
    {
	public:

		// Chest state filters
		config::Field<bool> f_FilterChestLocked;
		config::Field<bool> f_FilterChestInRock;
		config::Field<bool> f_FilterChestFrozen;
		config::Field<bool> f_FilterChestBramble;
		config::Field<bool> f_FilterChestTrap;

		// Chest rarity filters
		config::Field<bool> f_FilterChestCommon;
		config::Field<bool> f_FilterChestExquisite;
		config::Field<bool> f_FilterChestPrecious;
		config::Field<bool> f_FilterChestLuxurious;
		config::Field<bool> f_FilterChestRemarkable;

		// Type filters
		config::Field<bool> f_FilterChest;
		config::Field<bool> f_FilterInvestigates;
		config::Field<bool> f_FilterBookPage;
		config::Field<bool> f_FilterBGM;
		config::Field<bool> f_FilterQuestInt;
		config::Field<bool> f_FilterFloraChest;

		config::Field<bool> f_FilterUnknown;

		static ChestTeleport& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const final;
		virtual void DrawFilterOptions() final;

		void DrawItems() final;
		
		bool NeedInfoDraw() const final;
		void DrawInfo() final;

		bool IsValid(game::Entity* entity) const override;

		enum class FilterStatus
		{
			Unknown,
			Valid,
			Invalid
		};

		FilterStatus FilterChest(game::Chest* entity) const;
	private:

		void DrawChests();
		void DrawUnknowns();

		ChestTeleport();
	};
}

