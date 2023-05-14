NoCD.cpp

#include "pch-il2cpp.h"
#include "NoCD.h"

#include <helpers.h>
#include <fmt/chrono.h>

	namespace cheat::feature
{
	static bool HumanoidMoveFSM_CheckSprintCooldown_Hook(void *__this, MethodInfo *method);
	static bool LCAvatarCombat_IsEnergyMax_Hook(void *__this, MethodInfo *method);
	static bool LCAvatarCombat_OnSkillStart(app::LCAvatarCombat * __this, uint32_t skillID, float cdMultipler, MethodInfo *method);
	static bool LCAvatarCombat_IsSkillInCD_1(app::LCAvatarCombat * __this, app::LCAvatarCombat_LCAvatarCombat_SkillInfo * skillInfo, MethodInfo * method);

	static void ActorAbilityPlugin_AddDynamicFloatWithRange_Hook(app::MoleMole_ActorAbilityPlugin * __this, app::String * key, float value, float minValue, float maxValue,
																 bool forceDoAtRemote, MethodInfo *method);

	static std::list<std::string> abilityLog;

	NoCD::NoCD() : Feature(),
				   NFP(f_AbilityReduce, "NoCD", "Ability Reduce", false),
				   NF(f_TimerReduce, "NoCD", 1.f),
				   NFP(f_UtimateMaxEnergy, "NoCD", "Ultimate Max Energy", false),
				   NFP(f_Sprint, "NoCD", "Sprint", false),
				   NFP(f_InstantBow, "NoCD", "Instant Bow", false)
	{
		HookManager::install(app::MoleMole_LCAvatarCombat_IsEnergyMax, LCAvatarCombat_IsEnergyMax_Hook);
		HookManager::install(app::MoleMole_LCAvatarCombat_IsSkillInCD_1, LCAvatarCombat_IsSkillInCD_1);

		HookManager::install(app::MoleMole_HumanoidMoveFSM_CheckSprintCooldown, HumanoidMoveFSM_CheckSprintCooldown_Hook);
		HookManager::install(app::MoleMole_ActorAbilityPlugin_AddDynamicFloatWithRange, ActorAbilityPlugin_AddDynamicFloatWithRange_Hook);
	}

	const FeatureGUIInfo &NoCD::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Cooldown Effects", "Player");
		return info;
	}

	void NoCD::DrawMain()
	{
		ConfigWidget(_TR("Max Burst Energy"), f_UtimateMaxEnergy,
					 _TR("Removes energy requirement for elemental bursts.\n"
						 "(Energy bubble may appear incomplete but still usable.)"));

		ConfigWidget("## AbilityReduce", f_AbilityReduce);
		ImGui::SameLine();
		ConfigWidget(_TR("Reduce Skill/Burst Cooldown"), f_TimerReduce, 1.f, 1.f, 6.0f,
					 _TR("Reduce cooldowns of elemental skills and bursts.\n"
						 "1.0 - no CD, 2.0 and higher - increases the timer value."));

		ConfigWidget(_TR("No Sprint Cooldown"), f_Sprint, _TR("Removes delay in-between sprints."));

		ConfigWidget(_TR("Instant Bow Charge"), f_InstantBow, _TR("Disable cooldown of bow charge.\n"
																  "Known issues with Fischl."));

		if (f_InstantBow)
		{
			ImGui::Text(_TR("If Instant Bow Charge doesn't work:"));
			TextURL(_TR("Please report the issue on our Discord server."), "https://discord.com/channels/1069057220802781265/1075789562045018142", false, false);
			if (ImGui::TreeNode(_TR("Ability Log [DEBUG]")))
			{
				if (ImGui::Button(_TR("Copy to Clipboard")))
				{
					ImGui::LogToClipboard();

					ImGui::LogText(_TR("Ability Log:\n"));

					for (auto &logEntry : abilityLog)
						ImGui::LogText("%s\n", logEntry.c_str());

					ImGui::LogFinish();
				}

				for (const std::string &logEntry : abilityLog)
					ImGui::Text(logEntry.c_str());

				ImGui::TreePop();
			}
		}
	}

	bool NoCD::NeedStatusDraw() const
	{
		return f_InstantBow->enabled() || f_AbilityReduce->enabled() || f_Sprint->enabled();
	}

	void NoCD::DrawStatus()
	{
		std::string statusStr = _TR("Cooldown");
		if (f_AbilityReduce->enabled())
			statusStr += fmt::format(" {} x{:.1f}", _TR("Reduce"), f_TimerReduce.value());
		if (f_InstantBow->enabled())
		{
			if (f_AbilityReduce->enabled())
				statusStr += "|";
			statusStr += _TR("Bow");
		}
		if (f_Sprint->enabled())
		{
			if (f_AbilityReduce->enabled() || f_InstantBow->enabled())
				statusStr += "|";
			statusStr += _TR("Sprint");
		}
		ImGui::Text("%s", statusStr.c_str());
	}

	NoCD &NoCD::GetInstance()
	{
		static NoCD instance;
		return instance;
	}

	static bool LCAvatarCombat_IsEnergyMax_Hook(void *__this, MethodInfo *method)
	{
		NoCD &noCD = NoCD::GetInstance();
		if (noCD.f_UtimateMaxEnergy->enabled())
			return true;

		return CALL_ORIGIN(LCAvatarCombat_IsEnergyMax_Hook, __this, method);
	}

	// Multipler CoolDown Timer Old | RyujinZX#6666
	static bool LCAvatarCombat_OnSkillStart(app::LCAvatarCombat * __this, uint32_t skillID, float cdMultipler, MethodInfo *method)
	{
		NoCD &noCD = NoCD::GetInstance();
		if (noCD.f_AbilityReduce->enabled())
		{
			if (__this->fields._targetFixTimer->fields._._timer_k__BackingField > 0)
			{
				cdMultipler = noCD.f_TimerReduce / 3;
			}
			else
			{
				cdMultipler = noCD.f_TimerReduce / 1;
			}
		}
		return CALL_ORIGIN(LCAvatarCombat_OnSkillStart, __this, skillID, cdMultipler, method);
	}

	// Timer Speed Up / CoolDown Reduce New | RyujinZX#6666
	static bool LCAvatarCombat_IsSkillInCD_1(app::LCAvatarCombat * __this, app::LCAvatarCombat_LCAvatarCombat_SkillInfo * skillInfo, MethodInfo * method)
	{
		NoCD &noCD = NoCD::GetInstance();
		if (noCD.f_AbilityReduce->enabled())
		{
			auto cdTimer = app::MoleMole_SafeFloat_get_Value(skillInfo->fields.cdTimer, nullptr); // Timer start value in the game

			if (cdTimer > noCD.f_TimerReduce)
			{
				struct app::SafeFloat MyValueProtect = app::MoleMole_SafeFloat_set_Value(noCD.f_TimerReduce - 1.0f, nullptr); // Subtract -1 from the current timer value
				skillInfo->fields.cdTimer = MyValueProtect;
			}
		}
		return CALL_ORIGIN(LCAvatarCombat_IsSkillInCD_1, __this, skillInfo, method);
	}

	// Check sprint cooldown, we just return true if sprint no cooldown enabled.
	static bool HumanoidMoveFSM_CheckSprintCooldown_Hook(void *__this, MethodInfo *method)
	{
		NoCD &noCD = NoCD::GetInstance();
		if (noCD.f_Sprint->enabled())
			return true;

		return CALL_ORIGIN(HumanoidMoveFSM_CheckSprintCooldown_Hook, __this, method);
	}

	// This function is called when abilities with charge are charging, like a bow.
	// value - increase value
	// min and max - bounds of charge.
	// So, to charge make full charge m_Instantly, just replace value with maxValue.
	static void ActorAbilityPlugin_AddDynamicFloatWithRange_Hook(app::MoleMole_ActorAbilityPlugin * __this, app::String * key, float value, float minValue, float maxValue,
																 bool forceDoAtRemote, MethodInfo *method)
	{
		std::time_t t = std::time(nullptr);
		auto logEntry = fmt::format("{:%H:%M:%S} | Key: {} value: {} | min: {} | max: {}.", fmt::localtime(t), il2cppi_to_string(key), value, minValue, maxValue);
		abilityLog.push_front(logEntry);
		if (abilityLog.size() > 50)
			abilityLog.pop_back();

		NoCD &noCD = NoCD::GetInstance();
		// This function is called not only for bows, so if we don't put a key filter, it can cause various game mechanic bugs.
		// For now, only "_Enchanted_Time" is found for bow charging, maybe there are more. Need to continue research.
		if (noCD.f_InstantBow->enabled() && il2cppi_to_string(key) == "_Enchanted_Time")
		{
			value = maxValue;
			__this->fields.nextValidAbilityID = 36; // HotFix Yelan, Fishl | It's essentially a game bug. | RyujinZX#7832
		}

		CALL_ORIGIN(ActorAbilityPlugin_AddDynamicFloatWithRange_Hook, __this, key, value, minValue, maxValue, forceDoAtRemote, method);
	}
}