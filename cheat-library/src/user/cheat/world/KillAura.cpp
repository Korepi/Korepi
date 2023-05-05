#include "pch-il2cpp.h"
#include "KillAura.h"

#include <helpers.h>
#include <algorithm>

#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>
#include <cheat/game/filters.h>

namespace cheat::feature
{
	static void BaseMoveSyncPlugin_ConvertSyncTaskToMotionInfo_Hook(app::BaseMoveSyncPlugin* __this, MethodInfo* method);

	KillAura::KillAura() : Feature(),
		NFP(f_Enabled, "KillAura", "Kill Aura", false),
		NFP(f_DamageMode, "KillAura", "Damage mode", false),
		NFP(f_PercentDamageMode, "KillAura", "Percent damage mode", false),
		NFP(f_InstantDeathMode, "KillAura", "Instant death mode", false),
		NF(f_OnlyTargeted, "KillAura", true),
		NF(f_Range, "KillAura", 15.0f),
		NF(f_AttackDelay, "KillAura", 100),
		NF(f_RepeatDelay, "KillAura", 1000),
		NF(f_DamageValue, "KillAura", 233.0f),
		NF(f_PercentDamageTimes, "KillAura", 3)
	{
		events::GameUpdateEvent += MY_METHOD_HANDLER(KillAura::OnGameUpdate);
		HookManager::install(app::MoleMole_BaseMoveSyncPlugin_ConvertSyncTaskToMotionInfo, BaseMoveSyncPlugin_ConvertSyncTaskToMotionInfo_Hook);
	}

	const FeatureGUIInfo& KillAura::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Kill Aura", "World");
		return info;
	}

	void KillAura::DrawMain()
	{
		ConfigWidget(_TR("Enable Kill Aura"), f_Enabled, _TR("Enables kill aura. Need to choose a mode to work."));
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Choose any or both modes below."));

		ConfigWidget(_TR("Crash Damage Mode"), f_DamageMode, _TR("Kill aura causes crash damage for monster around you."));
		ImGui::Indent();
		ConfigWidget(_TR("Percent mode"), f_PercentDamageMode, _TR("Crash damage with percent value."));
		if (f_DamageMode->enabled())
		{
			if (!f_PercentDamageMode->enabled())
			{
				ConfigWidget(_TR("Damage Value"), f_DamageValue, 1, 0, 10000000, _TR("Crash damage value"));
			}
			else
			{
				ConfigWidget(_TR("Kill times"), f_PercentDamageTimes, 1, 1, 100, _TR("How many times to kill."));
			}
		}
		ImGui::Unindent();
		ConfigWidget(_TR("Instant Death Mode"), f_InstantDeathMode, _TR("Kill aura will attempt to instagib any valid target."));
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Can get buggy with bosses like PMA and Hydro Hypo."));
		ConfigWidget(_TR("Kill Range"), f_Range, 0.1f, 5.0f, 100.0f);
		ConfigWidget(_TR("Only Hostile/Aggro"), f_OnlyTargeted, _TR("If enabled, kill aura will only affect monsters targeting/aggro towards you."));
		ConfigWidget(_TR("Crash Attack Delay (ms)"), f_AttackDelay, 1, 0, 1000, _TR("Delay in ms before next crash damage."));
		ConfigWidget(_TR("Crash Repeat Delay (ms)"), f_RepeatDelay, 1, 100, 2000, _TR("Delay in ms before crash damaging same monster."));
	}

	bool KillAura::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}

	void KillAura::DrawStatus()
	{
		ImGui::Text("%s [%s%s]\n[%.01fm|%s|%dms|%dms]",
			_TR("Kill Aura"),
			f_DamageMode->enabled() && f_InstantDeathMode->enabled() ? _TR("Extreme") : f_DamageMode->enabled() ? _TR("Crash") : f_InstantDeathMode->enabled() ? _TR("Instant") : _TR("None"),
			f_DamageMode->enabled() ? !f_PercentDamageMode->enabled() ? _TR("|Fixed") : fmt::format("|{}({})", _TR("Rate"), f_PercentDamageTimes.value()).c_str() : "",
			f_Range.value(),
			f_OnlyTargeted ? _TR("Aggro") : _TR("All"),
			f_AttackDelay.value(),
			f_RepeatDelay.value());
	}

	KillAura& KillAura::GetInstance()
	{
		static KillAura instance;
		return instance;
	}

	// Kill aura logic is just emulate monster fall crash, so simple but works.
	// Note. No work on mob with shield, maybe update like auto ore destroy.
	void KillAura::OnGameUpdate()
	{
		static std::default_random_engine generator;
		static std::uniform_int_distribution<int> distribution(-50, 50);

		static int64_t nextAttackTime = 0;
		static std::map<uint32_t, int64_t> monsterRepeatTimeMap;
		static std::queue<game::Entity*> attackQueue;
		static std::unordered_set<uint32_t> attackSet;

		if (!f_Enabled->enabled() || !f_DamageMode->enabled())
			return;

		auto eventManager = GET_SINGLETON(MoleMole_EventManager);
		if (eventManager == nullptr || *app::MoleMole_EventHelper_Allocate_103__MethodInfo == nullptr)
			return;

		auto currentTime = util::GetCurrentTimeMillisec();
		if (currentTime < nextAttackTime)
			return;

		auto& manager = game::EntityManager::instance();

		for (const auto& monster : manager.entities(game::filters::combined::Monsters))
		{
			auto monsterID = monster->runtimeID();

			if (attackSet.count(monsterID) > 0)
				continue;

			if (monsterRepeatTimeMap.count(monsterID) > 0 && monsterRepeatTimeMap[monsterID] > currentTime)
				continue;

			auto combat = monster->combat();
			if (combat == nullptr)
				continue;

			auto combatProp = combat->fields._combatProperty_k__BackingField;
			if (combatProp == nullptr)
				continue;

			//auto maxHP = app::MoleMole_SafeFloat_get_Value(combatProp->fields.maxHP, nullptr);
			//auto isLockHp = combatProp->fields.islockHP == nullptr || app::MoleMole_FixedBoolStack_get_value(combatProp->fields.islockHP, nullptr);
			//auto isInvincible = combatProp->fields.isInvincible == nullptr || app::MoleMole_FixedBoolStack_get_value(combatProp->fields.isInvincible, nullptr);
			//auto HP = app::MoleMole_SafeFloat_get_Value(combatProp->fields.HP, nullptr);
			//if (maxHP < 10 || HP < 2 || isLockHp || isInvincible)
			//	continue;

			if (f_OnlyTargeted && combat->fields._attackTarget.runtimeID != manager.avatar()->runtimeID())
				continue;

			if (manager.avatar()->distance(monster) > f_Range)
				continue;

			attackQueue.push(monster);
			attackSet.insert(monsterID);
		}

		if (attackQueue.empty())
			return;

		auto monster = attackQueue.front();
		attackQueue.pop();

		if (!monster->isLoaded())
		{
			// If monster entity isn't active means that it was unloaded (it happen when player teleport or moving fast)
			// And we don't have way to get id
			// So better to clear all queue, to prevent memory leak
			// This happen rarely, so don't give any performance issues
			std::queue<game::Entity*> empty;
			std::swap(attackQueue, empty);

			attackSet.clear();
			return;
		}

		attackSet.erase(monster->runtimeID());

		auto combat = monster->combat();

		auto crashEvt = app::MoleMole_EventHelper_Allocate_103(*app::MoleMole_EventHelper_Allocate_103__MethodInfo);
		app::MoleMole_EvtCrash_Init(crashEvt, monster->runtimeID(), nullptr);

		if (!f_PercentDamageMode->enabled())
		{
			//Migita^Rin#1762: Fixed inaccurate damage caused by floating point precision(Maybe)
			float FPValue;
			if (f_DamageValue <= 10000000) FPValue = 27.0f;
			if (f_DamageValue <= 9000000) FPValue = 22.5f;
			if (f_DamageValue <= 8000000) FPValue = 20.0f;
			if (f_DamageValue <= 7000000) FPValue = 17.5f;
			if (f_DamageValue <= 6000000) FPValue = 15.0f;
			if (f_DamageValue <= 5000000) FPValue = 12.5f;
			if (f_DamageValue <= 4000000) FPValue = 10.0f;
			if (f_DamageValue <= 3000000) FPValue = 7.5f;
			if (f_DamageValue <= 2000000) FPValue = 5.0f;
			if (f_DamageValue <= 1000000) FPValue = 2.5f;

			crashEvt->fields.maxHp = f_DamageValue / 0.4f + FPValue;
			crashEvt->fields.velChange = 10000000;
		}
		else
		{
			auto maxHP = app::MoleMole_SafeFloat_get_Value(combat->fields._combatProperty_k__BackingField->fields.maxHP, nullptr);
			// should works. :p
			crashEvt->fields.maxHp = maxHP / float(f_PercentDamageTimes) / 0.4f + 5.0f;
			crashEvt->fields.velChange = 10000000;
		}
		crashEvt->fields.hitPos = monster->absolutePosition();

		app::MoleMole_EventManager_FireEvent(eventManager, reinterpret_cast<app::BaseEvent*>(crashEvt), false, nullptr);

		monsterRepeatTimeMap[monster->runtimeID()] = currentTime + (int)f_RepeatDelay + distribution(generator);

		nextAttackTime = currentTime + (int)f_AttackDelay + distribution(generator);
	}

	static void OnSyncTask(app::BaseMoveSyncPlugin* moveSync)
	{
		KillAura& killAura = KillAura::GetInstance();
		if (!killAura.f_Enabled->enabled() || !killAura.f_InstantDeathMode->enabled())
			return;

		auto& manager = game::EntityManager::instance();
		auto avatarID = manager.avatar()->runtimeID();
		auto entityID = moveSync->fields._.owner->fields.entityRuntimeID;

		if (entityID == avatarID)
			return;

		auto monster = manager.entity(entityID);
		auto combat = monster->combat();
		if (combat == nullptr)
			return;

		if (killAura.f_OnlyTargeted && combat->fields._attackTarget.runtimeID != avatarID)
			return;

		if (manager.avatar()->distance(monster) > killAura.f_Range)
			return;

		moveSync->fields._syncTask.position.x = 1000000.0f;
	}

	static void BaseMoveSyncPlugin_ConvertSyncTaskToMotionInfo_Hook(app::BaseMoveSyncPlugin* __this, MethodInfo* method)
	{
		OnSyncTask(__this);
		CALL_ORIGIN(BaseMoveSyncPlugin_ConvertSyncTaskToMotionInfo_Hook, __this, method);
	}
}

