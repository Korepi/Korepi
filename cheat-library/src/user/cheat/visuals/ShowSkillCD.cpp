#include "pch-il2cpp.h"
#include "ShowSkillCD.h"

#include <helpers.h>
#include <cheat/events.h>

namespace cheat::feature
{
	ShowSkillCD::ShowSkillCD() : Feature(),
		NF(f_Enabled, "Visuals::ShowSkillCD", false)
	{
		cheat::events::GameUpdateEvent += MY_METHOD_HANDLER(ShowSkillCD::OnGameUpdate);

		HookManager::install(app::MoleMole_LCAvatarCombat_SetSkillIndex, ShowSkillCD::MoleMole_LCAvatarCombat_SetSkillIndex_Hook);
		HookManager::install(app::MoleMole_LCAvatarCombat_CheckCDTimer, ShowSkillCD::MoleMole_LCAvatarCombat_CheckCDTimer_Hook);
		HookManager::install(app::MonoTeamBtn_SetupView, ShowSkillCD::MonoTeamBtn_SetupView_Hook);
	}

	const FeatureGUIInfo& ShowSkillCD::GetGUIInfo() const
	{
		TRANSLATED_MODULE_INFO("Visuals");
		return info;
	}

	void ShowSkillCD::DrawMain()
	{
		ConfigWidget(_TR("Show Skill Cooldowns"), f_Enabled, _TR("Show skill cooldowns and charges besides team buttons."));
	}

	bool ShowSkillCD::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void ShowSkillCD::DrawStatus()
	{
		ImGui::Text(_TR("Show Skill CDs"));
	}

	ShowSkillCD& ShowSkillCD::GetInstance()
	{
		static ShowSkillCD instance;
		return instance;
	}

	void ShowSkillCD::SetShowKey(void* key, bool show)
	{
		auto* component = reinterpret_cast<app::Component_1*>(key);
		auto* gameObject = app::Component_1_get_gameObject(component, nullptr);
		app::GameObject_set_active(gameObject, show, nullptr);
	}

	void ShowSkillCD::UpdateSkillMap(app::LCAvatarCombat* lcCombat, app::LCAvatarCombat_LCAvatarCombat_SkillInfo* skillInfo, float cd)
	{
		auto skillDepot = lcCombat->fields._skillDepotConfig->fields;
		uint32_t configID = app::MoleMole_SimpleSafeUInt32_get_Value(skillDepot.idRawNum, nullptr);
		uint32_t skillID = skillInfo->fields.skillID;

		auto& showSkillCD = GetInstance();
		showSkillCD.m_skillDataMap[configID].id = skillID;
		showSkillCD.m_skillDataMap[configID].cd = cd;
	}

	void ShowSkillCD::OnGameUpdate()
	{
		auto& showSkillCD = GetInstance();
		bool isPS4Layout = app::MoleMole_UIManager_IsPS4Layout(nullptr);

		for (auto it = showSkillCD.m_teamBtnMap.begin(), next_it = it; it != showSkillCD.m_teamBtnMap.end(); it = next_it)
		{
			++next_it;
			uint64_t guid = it->first;
			auto* teamBtn = it->second;
			if (teamBtn->fields._._._._._.m_CachedPtr == nullptr || 
				guid != teamBtn->fields._guid) // clean-up and prevent dupes from updating
			{
				showSkillCD.m_teamBtnMap.erase(it);
				continue;
			}

			SetShowKey(teamBtn->fields._pcKeyGrp, showSkillCD.f_Enabled ? true : !isPS4Layout);
			SetShowKey(app::MonoTeamBtn_get_PS4KeyIcon(teamBtn, nullptr), showSkillCD.f_Enabled ? false : isPS4Layout);

			if (showSkillCD.f_Enabled)
			{
				auto* lcCombat = teamBtn->fields._lcCombat;
				if (lcCombat != nullptr) // is nullptr when changing teams in abyss
				{
					auto* skillDepot = lcCombat->fields._skillDepotConfig;
					if (skillDepot != nullptr) // is nullptr when avatar has been removed from team
					{
						uint32_t configID = app::MoleMole_SimpleSafeUInt32_get_Value(skillDepot->fields.idRawNum, nullptr);
						uint32_t skillID = showSkillCD.m_skillDataMap[configID].id;

						float cd = showSkillCD.m_skillDataMap[configID].cd;
						int32_t maxCharge = app::MoleMole_LCAvatarCombat_GetSkillMaxChargesCount(lcCombat, skillID, nullptr);
						int32_t currCharge = app::MoleMole_LCAvatarCombat_GetSkillCurrentChargesCount(lcCombat, skillID, nullptr);

						bool ready = cd <= 0;
						std::string color = ready ? "green" : currCharge > 0 ? "olive" : "maroon";
						std::string cdStr = ready ? "Ready" : std::format("{:.1f}s", cd);
						std::string finalStr = std::format("<color={}>{}\n{:3}/{:<3}</color>", color, cdStr, currCharge, maxCharge);

						app::MonoTeamBtn_set_PCKey(teamBtn, string_to_il2cppi(finalStr), nullptr);
						continue;
					}
				}
				showSkillCD.m_teamBtnMap.erase(it);
			}
			else if (isPS4Layout) // minimize PCKey "blinking" effect in controller UI when feature is disabled
				app::MonoTeamBtn_set_PCKey(teamBtn, nullptr, nullptr);
		}
	}

	void ShowSkillCD::MoleMole_LCAvatarCombat_SetSkillIndex_Hook(app::LCAvatarCombat* __this, app::LCAvatarCombat_LCAvatarCombat_SkillInfo* skillInfo, int32_t index, int32_t priority, MethodInfo* method)
	{
		CALL_ORIGIN(MoleMole_LCAvatarCombat_SetSkillIndex_Hook, __this, skillInfo, index, priority, method);

		if (index == 1) // elemental skill index
			UpdateSkillMap(__this, skillInfo, 0);
	}

	void ShowSkillCD::MoleMole_LCAvatarCombat_CheckCDTimer_Hook(app::LCAvatarCombat* __this, app::LCAvatarCombat_LCAvatarCombat_SkillInfo* info, MethodInfo* method)
	{
		CALL_ORIGIN(MoleMole_LCAvatarCombat_CheckCDTimer_Hook, __this, info, method);

		if (info->fields.skillIndex == 1) // elemental skill index
		{
			float cd = app::MoleMole_SafeFloat_get_Value(info->fields.cdTimer, nullptr);
			UpdateSkillMap(__this, info, cd);
		}	
	}

	void ShowSkillCD::MonoTeamBtn_SetupView_Hook(app::MonoTeamBtn* __this, uint64_t guid, MethodInfo* method)
	{
		CALL_ORIGIN(MonoTeamBtn_SetupView_Hook, __this, guid, method);

		auto& showSkillCD = GetInstance();
		showSkillCD.m_teamBtnMap[guid] = __this;
	}
}