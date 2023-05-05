#include "pch-il2cpp.h"
#include "ChestTeleport.h"

#include <helpers.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/Chest.h>
#include <cheat/game/util.h>
#include <cheat/teleport/MapTeleport.h>

namespace cheat::feature 
{

    ChestTeleport::ChestTeleport() : ItemTeleportBase("ChestTeleport", "Chest"),
        NF(f_FilterChestLocked, "ChestTeleport", true),
		NF(f_FilterChestInRock, "ChestTeleport", true),
		NF(f_FilterChestFrozen, "ChestTeleport", true),
		NF(f_FilterChestBramble, "ChestTeleport", true),
		NF(f_FilterChestTrap, "ChestTeleport", true),

		NF(f_FilterChestCommon, "ChestTeleport", true),
		NF(f_FilterChestExquisite, "ChestTeleport", true),
		NF(f_FilterChestPrecious, "ChestTeleport", true),
		NF(f_FilterChestLuxurious, "ChestTeleport", true),
		NF(f_FilterChestRemarkable, "ChestTeleport", true),

		NF(f_FilterChest, "ChestTeleport", true),
		NF(f_FilterInvestigates, "ChestTeleport", false),
		NF(f_FilterBookPage, "ChestTeleport", false),
		NF(f_FilterBGM, "ChestTeleport", false),
		NF(f_FilterQuestInt, "ChestTeleport", false),
		NF(f_FilterFloraChest, "ChestTeleport", false),

		NF(f_FilterUnknown, "ChestTeleport", true)
	{ }


    void cheat::feature::ChestTeleport::DrawFilterOptions()
    {
		ConfigWidget(_TR("Show Info"), f_ShowInfo, _TR("Show short info about nearest chest to the info window."));

		if (ImGui::TreeNode(Translator::RuntimeTranslate("Filters").c_str()))
		{

			ImGui::Text(_TR("Type Filters"));

			ConfigWidget(_TR("Chests"), f_FilterChest, _TR("Enable chest detection and filters."));
			ConfigWidget(_TR("Investigates"), f_FilterInvestigates);
			ConfigWidget(_TR("Book pages"), f_FilterBookPage);
			ConfigWidget(_TR("BGMs"), f_FilterBGM);
			ConfigWidget(_TR("Quest Interaction"), f_FilterQuestInt);
			ConfigWidget(_TR("Flora chest"), f_FilterFloraChest);
			ConfigWidget(_TR("Unknowns"), f_FilterUnknown, _TR("Enable detection of unknown items.\nYou can see these items below, if they exist."));

			ImGui::Spacing();

			if (!f_FilterChest)
				ImGui::BeginDisabled();

			if (ImGui::BeginTable("Chest Filters", 2, ImGuiTableFlags_NoBordersInBody))
			{
				ImGui::TableNextColumn();
				ImGui::Text(_TR("Rarity Filter"));

				ImGui::TableNextColumn();
				ImGui::Text(_TR("State Filter"));

				ImGui::TableNextColumn();
				ConfigWidget(_TR("Common"), f_FilterChestCommon);

				ImGui::TableNextColumn();
				ConfigWidget(_TR("Locked"), f_FilterChestLocked);


				ImGui::TableNextColumn();
				ConfigWidget(_TR("Exquisite"), f_FilterChestExquisite);

				ImGui::TableNextColumn();
				ConfigWidget(_TR("In rock"), f_FilterChestInRock);


				ImGui::TableNextColumn();
				ConfigWidget(_TR("Precious"), f_FilterChestPrecious);

				ImGui::TableNextColumn();
				ConfigWidget(_TR("Frozen"), f_FilterChestFrozen);


				ImGui::TableNextColumn();
				ConfigWidget(_TR("Luxurious"), f_FilterChestLuxurious);

				ImGui::TableNextColumn();
				ConfigWidget(_TR("Bramble"), f_FilterChestBramble);


				ImGui::TableNextColumn();
				ConfigWidget(_TR("Remarkable"), f_FilterChestRemarkable);

				ImGui::TableNextColumn();
				ConfigWidget(_TR("Trap"), f_FilterChestTrap);

				ImGui::EndTable();
			}

			if (!f_FilterChest)
				ImGui::EndDisabled();

			ImGui::TreePop();
		}
    }

	const FeatureGUIInfo& ChestTeleport::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Chest Teleport", "Teleport");
		return info;
	}

    ChestTeleport& ChestTeleport::GetInstance()
	{
		static ChestTeleport instance;
		return instance;
	}

	bool ChestTeleport::IsValid(game::Entity* entity) const
	{
		if (!entity->isChest())
			return false;

		auto chest = dynamic_cast<game::Chest*>(entity);
		auto filterResult = FilterChest(chest);
		auto result = filterResult == ChestTeleport::FilterStatus::Valid ||
			(filterResult == ChestTeleport::FilterStatus::Unknown && f_FilterUnknown);
		return result;
	}

	cheat::feature::ChestTeleport::FilterStatus ChestTeleport::FilterChest(game::Chest* entity) const
	{
		auto itemType = entity->itemType();
		switch (itemType)
		{
		case game::Chest::ItemType::Chest:
		{
			if (!f_FilterChest)
				return FilterStatus::Invalid;
			
			auto chestRarity = entity->chestRarity();
			if (chestRarity == game::Chest::ChestRarity::Unknown)
				return FilterStatus::Unknown;
			
			bool rarityValid = (chestRarity == game::Chest::ChestRarity::Common && f_FilterChestCommon) ||
				(chestRarity == game::Chest::ChestRarity::Exquisite  && f_FilterChestExquisite) ||
				(chestRarity == game::Chest::ChestRarity::Precious   && f_FilterChestPrecious) ||
				(chestRarity == game::Chest::ChestRarity::Luxurious  && f_FilterChestLuxurious) ||
				(chestRarity == game::Chest::ChestRarity::Remarkable && f_FilterChestRemarkable);

			if (!rarityValid)
				return FilterStatus::Invalid;

			auto chestState = entity->chestState();
			if (chestState == game::Chest::ChestState::Invalid)
				return FilterStatus::Invalid;

			bool chestStateValid = chestState == game::Chest::ChestState::None ||
				(chestState == game::Chest::ChestState::Locked  && f_FilterChestLocked) ||
				(chestState == game::Chest::ChestState::InRock  && f_FilterChestInRock) ||
				(chestState == game::Chest::ChestState::Frozen  && f_FilterChestFrozen) ||
				(chestState == game::Chest::ChestState::Bramble && f_FilterChestBramble) ||
				(chestState == game::Chest::ChestState::Trap    && f_FilterChestTrap);

			if (!chestStateValid)
				return FilterStatus::Invalid;

			return FilterStatus::Valid;
		}
		case game::Chest::ItemType::Investigate:
			return f_FilterInvestigates ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::BookPage:
			return f_FilterBookPage ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::BGM:
			return f_FilterBGM ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::QuestInteract:
			return f_FilterQuestInt ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::Flora:
			return f_FilterFloraChest ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::None:
		default:
			return FilterStatus::Unknown;
		}

		return FilterStatus::Unknown;
	}

	void ChestTeleport::DrawItems()
	{
		DrawUnknowns();
		DrawChests();
	}

	bool ChestTeleport::NeedInfoDraw() const
	{
		return true;
	}

	void ChestTeleport::DrawInfo()
	{
		auto entity = game::FindNearestEntity(*this);
		auto chest = dynamic_cast<game::Chest*>(entity);

		DrawEntityInfo(entity);
		if (entity == nullptr)
			return;
		ImGui::SameLine();

		ImGui::TextColored(chest->chestColor(), "%s", chest->minName().c_str());
	}

	void ChestTeleport::DrawChests()
	{	
		if (!ImGui::TreeNode(_TR("Items")))
			return;

		auto& manager = game::EntityManager::instance();
		auto entities = manager.entities(*this);

		ImGui::BeginTable("ChestsTable", 2);
		for (auto entity : entities)
		{
			ImGui::PushID(entity);
			auto chest = dynamic_cast<game::Chest*>(entity);

			ImGui::TableNextColumn();
			if (chest->itemType() == game::Chest::ItemType::Chest)
			{
				ImGui::TextColored(chest->chestColor(), "%s [%s] [%s] at %0.3fm", 
					magic_enum::enum_name(chest->itemType()).data(),
					magic_enum::enum_name(chest->chestRarity()).data(),
					magic_enum::enum_name(chest->chestState()).data(),
					manager.avatar()->distance(entity));
			}
			else
			{
				ImGui::TextColored(chest->chestColor(), "%s at %0.3fm", magic_enum::enum_name(chest->itemType()).data(), 
					manager.avatar()->distance(entity));
			}

			ImGui::TableNextColumn();

			if (ImGui::Button(_TR("Teleport")))
			{
				auto& mapTeleport = MapTeleport::GetInstance();
				mapTeleport.TeleportTo(chest->absolutePosition());
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
		ImGui::TreePop();
	}

	static bool ChestUnknownFilter(game::Entity* entity)
	{
		if (!entity->isChest())
			return false;

		auto chest = dynamic_cast<game::Chest*>(entity);
		auto& chestTp = ChestTeleport::GetInstance();
		return chestTp.FilterChest(chest) == ChestTeleport::FilterStatus::Unknown;
	}

	void ChestTeleport::DrawUnknowns()
	{
		auto& manager = game::EntityManager::instance();
		auto unknowns = manager.entities(ChestUnknownFilter);
		if (unknowns.empty())
			return;

		ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Wow, you found unknown chest names. This means this chest type has no filter yet."));
		TextURL(_TR("Please report the issue on our Discord server."), "https://discord.com/channels/1069057220802781265/1075789562045018142", false, false);
		
		if (ImGui::Button(_TR("Copy to Clipboard")))
		{
			ImGui::LogToClipboard();

			ImGui::LogText(_TR("Unknown Names:\n"));
			
			for (auto& entity : unknowns)
				ImGui::LogText(_TR("%s; Position: %s; Scene: %u\n"), entity->name().c_str(), 
					il2cppi_to_string(entity->relativePosition()).c_str(), game::GetCurrentPlayerSceneID());
			
			ImGui::LogFinish();
		}

		if (!ImGui::TreeNode(_TR("Unknown Items")))
			return;

		ImGui::BeginTable("UnknownsTable", 2);

		for (auto& entity : unknowns)
		{
			ImGui::PushID(entity);
			
			ImGui::TableNextColumn();
			ImGui::Text(_TR("%s. Dist %0.3f"), entity->name().c_str(), manager.avatar()->distance(entity));
			
			ImGui::TableNextColumn();
			if (ImGui::Button(_TR("TP")))
			{
				auto& mapTeleport = MapTeleport::GetInstance();
				mapTeleport.TeleportTo(entity->absolutePosition());
			}

			ImGui::PopID();
		}
		ImGui::EndTable();
		ImGui::TreePop();
	}
}
