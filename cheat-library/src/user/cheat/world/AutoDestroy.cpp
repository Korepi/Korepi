#include "pch-il2cpp.h"
#include "AutoDestroy.h"

#include <helpers.h>
#include <algorithm>

#include <cheat/events.h>
#include <cheat/game/SimpleFilter.h>
#include <cheat/game/EntityManager.h>
#include <cheat/world/AutoChallenge.h>
#include <cheat/game/filters.h>

namespace cheat::feature
{
	static void LCAbilityElement_ReduceModifierDurability_Hook(app::LCAbilityElement* __this, int32_t modifierDurabilityIndex, float reduceDurability, app::Nullable_1_Single_ deltaTime, MethodInfo* method);

	AutoDestroy::AutoDestroy() : Feature(),
		NFP(f_Enabled, "AutoDestroy", "Auto destroy", false),
		NFP(f_DestroyOres, "AutoDestroy", "Destroy ores", false),
		NFP(f_DestroyShields, "AutoDestroy", "Destroy shields", false),
		NFP(f_DestroyDoodads, "AutoDestroy", "Destroy doodads", false),
		NFP(f_DestroyPlants, "AutoDestroy", "Destroy plants", false),
		NFP(f_DestroySpecialObjects, "AutoDestroy", "Destroy Specs", false),
		NFP(f_DestroySpecialChests, "AutoDestroy", "Destroy Spec Chests", false),
		NF(f_Range, "AutoDestroy", 10.0f)
	{
		HookManager::install(app::MoleMole_LCAbilityElement_ReduceModifierDurability, LCAbilityElement_ReduceModifierDurability_Hook);
	}

	const FeatureGUIInfo& AutoDestroy::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Auto Destroy Objects", "World");
		return info;
	}

	void AutoDestroy::DrawMain()
	{
		ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Note. This feature is not fully tested detection-wise.\n"
			"Not recommended for main accounts or used with high values."));

		ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Instantly destroys non-living objects within range."));
		ImGui::Indent();
		ConfigWidget(_TR("Ores"), f_DestroyOres, _TR("Ores and variants, e.g. electro crystals, marrows, etc."));
		ConfigWidget(_TR("Shields"), f_DestroyShields, _TR("Abyss mage/churl/slime shields."));
		ConfigWidget(_TR("Doodads"), f_DestroyDoodads, _TR("Barrels, boxes, vases, etc."));
		ConfigWidget(_TR("Plants"), f_DestroyPlants, _TR("Dandelion Seeds, Sakura Bloom, etc."));
		ConfigWidget(_TR("Special Objects"), f_DestroySpecialObjects, _TR("Destroy Ancient Rime, Large and Small Rock Piles"));
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Risk Unknown!"));
		ConfigWidget(_TR("Special Chests"), f_DestroySpecialChests, _TR("Destroy Chests with Brambles, Frozen, or In Rocks"));
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Risk Unknown!"));
		ImGui::Unindent();
		ConfigWidget(_TR("Range (m)"), f_Range, 0.1f, 1.0f, 15.0f);
	}

	bool AutoDestroy::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}

	void AutoDestroy::DrawStatus()
	{
		ImGui::Text("%s[%.01fm%s%s%s%s%s%s%s]",
			_TR("Destroy"),
			f_Range.value(),
			f_DestroyOres->enabled() || f_DestroyShields->enabled() || f_DestroyDoodads->enabled() || f_DestroyPlants->enabled() || f_DestroySpecialObjects->enabled() || f_DestroySpecialChests->enabled() ? "|" : "",
			f_DestroyOres->enabled() ? "O" : "",
			f_DestroyShields->enabled() ? "S" : "",
			f_DestroyDoodads->enabled() ? "D" : "",
			f_DestroyPlants->enabled() ? "P" : "",
			f_DestroySpecialObjects->enabled() ? "SO" : "",
			f_DestroySpecialChests->enabled() ? "SC" : "");
	}

	AutoDestroy& AutoDestroy::GetInstance()
	{
		static AutoDestroy instance;
		return instance;
	}

	// Thanks to @RyujinZX
	// Every ore has ability element component
	// Durability of ability element is a ore health
	// Every tick ability element check reducing durability, for ore in calm state `reduceDurability` equals 0, means HP don't change
	// We need just change this value to current durability or above to destroy ore
	// This function also can work with some types of shields (TODO: improve killaura with this function)
	static void LCAbilityElement_ReduceModifierDurability_Hook(app::LCAbilityElement* __this, int32_t modifierDurabilityIndex, float reduceDurability, app::Nullable_1_Single_ deltaTime, MethodInfo* method)
	{
		auto& manager = game::EntityManager::instance();
		auto& autoDestroy = AutoDestroy::GetInstance();
		auto& autoChallenge = AutoChallenge::GetInstance();
		auto entity = __this->fields._._._entity;
		// call origin ReduceModifierDurability without correct modifierDurabilityIndex will coz game crash.
		// so use this hook function to destroy challenge's bombbarrel
		if (autoChallenge.f_Enabled->enabled() && autoChallenge.f_BombDestroy->enabled() &&
			autoChallenge.f_Range > manager.avatar()->distance(entity) &&
			game::filters::puzzle::Bombbarrel.IsValid(manager.entity(entity))
			)
		{
			reduceDurability = 1000.f;
		}
		if (autoDestroy.f_Enabled->enabled() &&
			autoDestroy.f_Range > manager.avatar()->distance(entity) &&
			(
				(autoDestroy.f_DestroyOres->enabled() && game::filters::combined::Ores.IsValid(manager.entity(entity))) ||
				(autoDestroy.f_DestroyDoodads->enabled() && (game::filters::combined::Doodads.IsValid(manager.entity(entity)) || game::filters::chest::SBramble.IsValid(manager.entity(entity)))) ||
				(autoDestroy.f_DestroyShields->enabled() && !game::filters::combined::MonsterBosses.IsValid(manager.entity(entity)) && (
					game::filters::combined::MonsterShielded.IsValid(manager.entity(entity)) ||											// For shields attached to monsters, e.g. abyss mage shields.
					game::filters::combined::MonsterEquips.IsValid(manager.entity(entity)))) ||											// For shields/weapons equipped by monsters, e.g. rock shield.
					(autoDestroy.f_DestroyPlants->enabled() && game::filters::combined::PlantDestroy.IsValid(manager.entity(entity))) ||			// For plants e.g dandelion seeds.
				(autoDestroy.f_DestroySpecialObjects->enabled() && game::filters::combined::BreakableObjects.IsValid(manager.entity(entity))) ||	// For Breakable Objects e.g Ancient Rime, Large and Small Rock Piles.
				(autoDestroy.f_DestroySpecialChests->enabled() && game::filters::combined::Chests.IsValid(manager.entity(entity)))					// For Special Chests e.g Brambles, Frozen, Encased in Rock.
				)
			)
		{
			// This value always above any ore durability
			reduceDurability = 1000;
		}
		CALL_ORIGIN(LCAbilityElement_ReduceModifierDurability_Hook, __this, modifierDurabilityIndex, reduceDurability, deltaTime, method);
	}

}

