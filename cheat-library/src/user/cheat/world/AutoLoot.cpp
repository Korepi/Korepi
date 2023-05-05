#include "pch-il2cpp.h"
#include "AutoLoot.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/filters.h>
#include <cheat/game/Chest.h>

namespace cheat::feature 
{
	static void ItemModule_OnCheckAddItemExceedLimitNotify_Hook(/*MoleMole_ItemModule* */ void* __this, app::Proto_CheckAddItemExceedLimitNotify* notify, MethodInfo* method);
	static void LCSelectPickup_AddInteeBtnByID_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method);
	static bool LCSelectPickup_IsInPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method);
	static bool LCSelectPickup_IsOutPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method);

	float g_default_range = 3.0f;

    AutoLoot::AutoLoot() : Feature(),
        NFP(f_AutoPickup, "AutoLoot", "Auto-Pickup", false),
        NFP(f_AutoDisablePickupWhenAddItemExceedLimit, "AutoLoot", "Auto disable pickup (on full)", true),
        NFP(f_AutoTreasure, "AutoLoot", "Auto-Treasure", false),
        NFP(f_UseCustomRange, "AutoLoot", "Use custom range", false),
        NFP(f_UseDelayTimeFluctuation, "AutoLoot", "Use delay time fluctuation", false),
        NFP(f_PickupFilter, "AutoLoot", "Pickup Filter", false),
        NF(f_DelayTime, "AutoLoot", 200),
        NF(f_DelayTimeFluctuation, "AutoLoot", 200),
        NF(f_CustomRange, "AutoLoot", 5.0f),
        NF(f_Chest, "AutoLoot", false),
        NF(f_Leyline, "AutoLoot", false),
        NF(f_Investigate, "AutoLoot", false),
        NF(f_QuestInteract, "AutoLoot", false),
        NF(f_Others, "AutoLoot", false),
        NF(f_PickupFilter_Animals, "AutoLoot", true),
        NF(f_PickupFilter_DropItems, "AutoLoot", true),
        NF(f_PickupFilter_Resources, "AutoLoot", true),
        NF(f_PickupFilter_Oculus, "AutoLoot", true),
        toBeLootedItems(), nextLootTime(0)
    {
		// Auto loot
		HookManager::install(app::MoleMole_LCSelectPickup_AddInteeBtnByID, LCSelectPickup_AddInteeBtnByID_Hook);
		HookManager::install(app::MoleMole_LCSelectPickup_IsInPosition, LCSelectPickup_IsInPosition_Hook);
		HookManager::install(app::MoleMole_LCSelectPickup_IsOutPosition, LCSelectPickup_IsOutPosition_Hook);
		HookManager::install(app::MoleMole_ItemModule_OnCheckAddItemExceedLimitNotify, ItemModule_OnCheckAddItemExceedLimitNotify_Hook);

		events::GameUpdateEvent += MY_METHOD_HANDLER(AutoLoot::OnGameUpdate);
	}

    const FeatureGUIInfo& AutoLoot::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Auto Loot", "World");
        return info;
    }

    void AutoLoot::DrawMain()
    {
		if (ImGui::BeginTable("AutoLootDrawTable", 2, ImGuiTableFlags_NoBordersInBody))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::BeginGroupPanel(_TR("Auto-Pickup"));
			{
				ConfigWidget(_TR("Enabled"), f_AutoPickup, _TR("Automatically picks up dropped items.\n" \
					"Note: Using this with custom range and low delay times is extremely risky.\n" \
					"Abuse will definitely merit a ban.\n\n" \
					"If using with custom range, make sure this is turned on FIRST."));
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Read the note!"));
				ImGui::Indent();
				ConfigWidget(_TR("Auto disable when bag is full"), f_AutoDisablePickupWhenAddItemExceedLimit, _TR("Automatically disables auto pickup when bag is full."));
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Read the note!"));
			}
			ImGui::EndGroupPanel();
			
			ImGui::BeginGroupPanel(_TR("Custom Pickup Range"));
			{
				ConfigWidget(_TR("Enabled"), f_UseCustomRange, _TR("Enable custom pickup range.\n" \
					"High values are not recommended, as it is easily detected by the server.\n\n" \
					"If using with auto-pickup/auto-treasure, turn this on LAST."));
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Read the note!"));
				ImGui::SetNextItemWidth(100.0f);
				ConfigWidget(_TR("Range (m)"), f_CustomRange, 0.1f, 0.5f, 40.0f, _TR("Modifies pickup/open range to this value (in meters)."));
			}
			ImGui::EndGroupPanel();
			
			ImGui::BeginGroupPanel(_TR("Looting Speed"));
			{
				ImGui::SetNextItemWidth(100.0f);
				ConfigWidget(_TR("Delay Time (ms)"), f_DelayTime, 1, 0, 1000, _TR("Delay (in ms) between loot/open actions.\n" \
					"Values under 200ms are unsafe.\nNot used if no auto-functions are on."));
			}
			ImGui::EndGroupPanel();

			ImGui::BeginGroupPanel(_TR("Looting delay fluctuation"));
			{
				ConfigWidget(_TR("Enabled"), f_UseDelayTimeFluctuation, _TR("Enable delay fluctuation.\n" \
					"Simulates human clicking delay as manual clickling never consistent."));
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Read the note!"));
				ImGui::SetNextItemWidth(100.0f);
				ConfigWidget(_TR("Delay range +(ms)"), f_DelayTimeFluctuation, 1, 0, 1000, _TR("Delay randomly fluctuates between 'Delay Time'+'Delay Time+range'"));
			}
			ImGui::EndGroupPanel();
			
			ImGui::TableSetColumnIndex(1);
			ImGui::BeginGroupPanel(_TR("Auto-Treasure"));
			{
				ConfigWidget(_TR("Enabled"), f_AutoTreasure, _TR("Automatically opens chests and other treasures.\n" \
					"Note: Using this with custom range and low delay times is extremely risky.\n" \
					"Abuse will definitely merit a ban.\n\n" \
					"If using with custom range, make sure this is turned on FIRST."));
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Read the note!"));
				ImGui::Indent();
				ConfigWidget(_TR("Chests"), f_Chest, _TR("Common, precious, luxurious, etc."));
				ConfigWidget(_TR("Leyline"), f_Leyline, _TR("Mora/XP, overworld/Trounce bosses, etc."));
				ConfigWidget(_TR("Search Points"), f_Investigate, _TR("Marked as Investigate/Search, etc."));
				ConfigWidget(_TR("Quest Interacts"), f_QuestInteract, _TR("Valid quest interact points."));
				ConfigWidget(_TR("Others"), f_Others, _TR("Book Pages, Spincrystals, etc."));
				ImGui::Unindent();
			}
			ImGui::EndGroupPanel();
			ImGui::EndTable();
		}
			
    	ImGui::BeginGroupPanel(_TR("Pickup Filter"));
	    {
			ConfigWidget(_TR("Enabled"), f_PickupFilter, _TR("Enable pickup filter.\n"));
			ConfigWidget(_TR("Animals"), f_PickupFilter_Animals, _TR("Fish, Lizard, Frog, Flying animals.")); ImGui::SameLine();
			ConfigWidget(_TR("Drop Items"), f_PickupFilter_DropItems, _TR("Material, Mineral, Artifact.")); ImGui::SameLine();
			ConfigWidget(_TR("Resources"), f_PickupFilter_Resources, _TR("Everything beside Animals and Drop Items (Plants, Books, etc).")); ImGui::SameLine();
			ConfigWidget(_TR("Oculus"), f_PickupFilter_Oculus, _TR("Filter Oculus"));
	    }
    	ImGui::EndGroupPanel();
    }

    bool AutoLoot::NeedStatusDraw() const
	{
        return f_AutoPickup->enabled() || f_AutoTreasure->enabled() || f_UseCustomRange->enabled() || f_PickupFilter->enabled();
    }

    void AutoLoot::DrawStatus() 
    {
		std::string s;
		// Auto Pickup
		if (f_AutoPickup->enabled()) {
			s.append(_TR("AP"));
			if (f_AutoDisablePickupWhenAddItemExceedLimit->enabled())
				s.append("|")
				.append(_TR("AD"));
		}
		// Auto Treasure
		if (f_AutoTreasure->enabled())
			s.append(s.empty() ? "" : "|")
			.append(_TR("AT"));

		// Custom Range
		if (f_UseCustomRange->enabled())
			s.append(s.empty() ? "" : "|")
			.append(_TR("CR"))
			.append(fmt::format("{:.1f}m", f_CustomRange.value()));
	
		// Pickup Filter
		if (f_PickupFilter->enabled())
			s.append(s.empty() ? "" : "|")
			.append(_TR("PF"));
		
		// Pickup Delay
		if (f_AutoPickup->enabled() || f_AutoTreasure->enabled())
			s.append(fmt::format("|{}ms", f_DelayTime.value()))
			.append(f_UseDelayTimeFluctuation->enabled() ? fmt::format("|{}+{}ms", _TR("FL"), f_DelayTimeFluctuation.value()) : "");
		
		ImGui::Text("%s\n[%s]",
			_TR("Auto Loot"),
			s.c_str()
		);
    }

    AutoLoot& AutoLoot::GetInstance()
    {
        static AutoLoot instance;
        return instance;
    }

	bool AutoLoot::OnCreateButton(app::BaseEntity* entity)
	{
		if (!f_AutoPickup->enabled())
			return false;

		auto itemModule = GET_SINGLETON(MoleMole_ItemModule);
		if (itemModule == nullptr)
			return false;
    	
		auto entityId = entity->fields._runtimeID_k__BackingField;
		if (f_DelayTime == 0)
		{
			app::MoleMole_ItemModule_PickItem(itemModule, entityId, nullptr);
			return true;
		}

		toBeLootedItems.push(entityId);
		return false;
	}

	void AutoLoot::OnGameUpdate()
	{
		auto currentTime = util::GetCurrentTimeMillisec();
		if (currentTime < nextLootTime)
			return;

		auto entityManager = GET_SINGLETON(MoleMole_EntityManager);
		if (entityManager == nullptr)
			return;

		// RyujinZX#6666
		if (f_AutoTreasure->enabled()) 
		{
			auto& manager = game::EntityManager::instance();
			for (auto entity : manager.entities(game::filters::combined::Chests)) 
			{
				float range = f_UseCustomRange->enabled() ? f_CustomRange : g_default_range;
				if (manager.avatar()->distance(entity) >= range)
					continue;

				auto chest = dynamic_cast<game::Chest*>(entity);
				auto chestType = chest->itemType();

				if (!f_Investigate && chestType == game::Chest::ItemType::Investigate)
					continue;

				if (!f_QuestInteract && chestType == game::Chest::ItemType::QuestInteract)
					continue;

				if (!f_Others && (
					chestType == game::Chest::ItemType::BGM ||
					chestType == game::Chest::ItemType::BookPage
					))
					continue;

				if (!f_Leyline && chestType == game::Chest::ItemType::Flora)
					continue;

				if (chestType == game::Chest::ItemType::Chest)
				{
					if (!f_Chest)
						continue;
					auto ChestState = chest->chestState();
					if (ChestState != game::Chest::ChestState::None)
						continue;
				}

				uint32_t entityId = entity->runtimeID();
				toBeLootedItems.push(entityId);
			}
		}

		auto entityId = toBeLootedItems.pop();
		if (!entityId)
			return;

		auto itemModule = GET_SINGLETON(MoleMole_ItemModule);
		if (itemModule == nullptr)
			return;

		auto entity = app::MoleMole_EntityManager_GetValidEntity(entityManager, *entityId, nullptr);
		if (entity == nullptr)
			return;

		app::MoleMole_ItemModule_PickItem(itemModule, *entityId, nullptr);

		int fluctuation = 0;
		if (f_UseDelayTimeFluctuation->enabled())
		{
			fluctuation = std::rand() % (f_DelayTimeFluctuation + 1);
		}

		nextLootTime = currentTime + (int)f_DelayTime + fluctuation;
	}

	void AutoLoot::OnCheckIsInPosition(bool& result, app::BaseEntity* entity)
	{
		// TODO: Maybe add a list of filter for all GatherObject instead of just using entityType in general.
		auto& manager = game::EntityManager::instance();

		if (f_AutoPickup->enabled() || f_UseCustomRange->enabled()) {
			float pickupRange = f_UseCustomRange->enabled() ? f_CustomRange : g_default_range;
			if (f_PickupFilter->enabled())
			{
				if (!f_PickupFilter_Animals && entity->fields.entityType == app::EntityType__Enum_1::EnvAnimal ||
					!f_PickupFilter_DropItems && entity->fields.entityType == app::EntityType__Enum_1::DropItem ||
					!f_PickupFilter_Resources && entity->fields.entityType == app::EntityType__Enum_1::GatherObject ||
					!f_PickupFilter_Oculus && game::filters::combined::Oculies.IsValid(manager.entity(entity->fields._runtimeID_k__BackingField)))
				{
					result = false;
					return;
				}
			}
			
			result = manager.avatar()->distance(entity) < pickupRange;
		}
	}

	static void LCSelectPickup_AddInteeBtnByID_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method)
	{
		AutoLoot& autoLoot = AutoLoot::GetInstance();
		bool canceled = autoLoot.OnCreateButton(entity);
		if (!canceled)
			CALL_ORIGIN(LCSelectPickup_AddInteeBtnByID_Hook, __this, entity, method);
	}

	static bool LCSelectPickup_IsInPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method)
	{
		bool result = CALL_ORIGIN(LCSelectPickup_IsInPosition_Hook, __this, entity, method);

		AutoLoot& autoLoot = AutoLoot::GetInstance();
		autoLoot.OnCheckIsInPosition(result, entity);

		return result;
	}

	static bool LCSelectPickup_IsOutPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method)
	{
		bool result = CALL_ORIGIN(LCSelectPickup_IsOutPosition_Hook, __this, entity, method);

		AutoLoot& autoLoot = AutoLoot::GetInstance();
		autoLoot.OnCheckIsInPosition(result, entity);

		return result;
	}

	void AutoLoot::clear_toBeLootedItems()
	{
		for (uint32_t i = 0; i < toBeLootedItems.size(); ++i)
		{
			toBeLootedItems.pop();
		}
	}

	static void ItemModule_OnCheckAddItemExceedLimitNotify_Hook(/*MoleMole_ItemModule* */ void*  __this, app::Proto_CheckAddItemExceedLimitNotify* notify, MethodInfo* method)
	{
		AutoLoot& autoLoot = AutoLoot::GetInstance();
		if (autoLoot.f_AutoPickup->enabled() && autoLoot.f_AutoDisablePickupWhenAddItemExceedLimit->enabled() &&
			// notify->fields.reason_ != 1103 // ACTION_REASON_HOME_PLANT_BOX_GATHER = 0x44F, alternative to only exclude seed box gather if below condition has problems
			notify->fields.msgType_ != app::Proto_CheckAddItemExceedLimitNotify_ItemExceedLimitMsgType__Enum::ITEM_EXCEED_LIMIT_MSG_TYPE_TEXT) // exclude if prompt is only text
		{
			// Temporary toggles autoloot off, not saved to config
			autoLoot.f_AutoPickup->set_enabled(false);

			// Clear loot queue
			autoLoot.clear_toBeLootedItems();
		}

		return CALL_ORIGIN(ItemModule_OnCheckAddItemExceedLimitNotify_Hook, __this, notify, method);
	}
}

