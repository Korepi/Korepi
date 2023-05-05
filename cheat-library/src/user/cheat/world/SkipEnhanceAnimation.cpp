#include "pch-il2cpp.h"
#include "SkipEnhanceAnimation.h"

#include "helpers.h"

namespace cheat::feature
{
	static void MoleMole_EquipLevelUpDialogContext_SetupView_Hook(app::MoleMole_EquipLevelUpDialogContext* __this, MethodInfo* method);
	static void MoleMole_EquipOverviewPageContext_PlayExpAddAnimation_Hook(/*MoleMole_EquipOverviewPageContext*/ void* __this, float startPer, float endPer, /*Action*/ void* callback, MethodInfo* method);
	static void MoleMole_EquipOverviewPageContext_PlayLevelUpSuccessShow_Hook(/*MoleMole_EquipOverviewPageContext*/ void* __this, /*Action*/ void* refreshViewAction, MethodInfo* method);

	static uint32_t substatRollLevels[] = { 5, 9, 13, 17, 21 }; // artifact levels from the field go from 1 to 21, so we do +1

	SkipEnhanceAnimation::SkipEnhanceAnimation() : Feature(),
		NF(f_Enabled, "SkipEnhanceAnimation", false),
		NF(f_ShowLevelUp, "SkipEnhanceAnimation", true)
	{
		HookManager::install(app::MoleMole_EquipLevelUpDialogContext_SetupView, MoleMole_EquipLevelUpDialogContext_SetupView_Hook);
		HookManager::install(app::MoleMole_EquipOverviewPageContext_PlayExpAddAnimation, MoleMole_EquipOverviewPageContext_PlayExpAddAnimation_Hook);
		HookManager::install(app::MoleMole_EquipOverviewPageContext_PlayLevelUpSuccessShow, MoleMole_EquipOverviewPageContext_PlayLevelUpSuccessShow_Hook);
	}

	const FeatureGUIInfo& SkipEnhanceAnimation::GetGUIInfo() const
	{
		TRANSLATED_MODULE_INFO("World");
		return info;
	}

	void SkipEnhanceAnimation::DrawMain()
	{
		ConfigWidget(_TR("Skip Enhance Animation"), f_Enabled, _TR("Skip weapon and artifact enhancement animation."));
		if (f_Enabled)
		{
			ImGui::Indent();
			ConfigWidget(_TR("Show Level-Up Dialog For Substat Rolls"), f_ShowLevelUp,
				_TR("Show level up dialog when artifacts roll substats\n"
				"(when hitting levels 4, 8, 12, 16, and 20)."));
			ImGui::Unindent();
		}
	}

	bool SkipEnhanceAnimation::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void SkipEnhanceAnimation::DrawStatus()
	{
		ImGui::Text(_TR("Skip Enhance Animation"));
	}

	SkipEnhanceAnimation& SkipEnhanceAnimation::GetInstance()
	{
		static SkipEnhanceAnimation instance;
		return instance;
	}

	bool SkipEnhanceAnimation::ShouldShowLevelUpDialog(app::MoleMole_EquipLevelUpDialogContext* dialog)
	{
		auto& skipEnhaceAnimation = GetInstance();
		if (skipEnhaceAnimation.f_ShowLevelUp)
		{
			if (dialog->fields._equipType == app::MoleMole_Config_ItemType__Enum::ITEM_RELIQUARY)
			{
				const uint32_t prevLvl = dialog->fields._prevLevel;
				const uint32_t currLvl = dialog->fields._currLevel;
				return std::any_of(std::begin(substatRollLevels), std::end(substatRollLevels),
								   [prevLvl, currLvl](uint32_t level) { return prevLvl < level && level <= currLvl; });
			}
		}
		return false;
	}

	void MoleMole_EquipLevelUpDialogContext_SetupView_Hook(app::MoleMole_EquipLevelUpDialogContext* __this, MethodInfo* method)
	{
		CALL_ORIGIN(MoleMole_EquipLevelUpDialogContext_SetupView_Hook, __this, method);

		auto& skipEnhaceAnimation = SkipEnhanceAnimation::GetInstance();
		if (skipEnhaceAnimation.f_Enabled && !skipEnhaceAnimation.ShouldShowLevelUpDialog(__this))
			app::MoleMole_EquipLevelUpDialogContext_ShowReturnedMaterialAndCloseDialog(__this, method);
	}

	void MoleMole_EquipOverviewPageContext_PlayExpAddAnimation_Hook(/*MoleMole_EquipOverviewPageContext*/ void* __this, float startPer, float endPer, /*Action*/ void* callback, MethodInfo* method)
	{
		auto& skipEnhaceAnimation = SkipEnhanceAnimation::GetInstance();
		if (skipEnhaceAnimation.f_Enabled)
			return app::Action_Invoke(callback, method);

		CALL_ORIGIN(MoleMole_EquipOverviewPageContext_PlayExpAddAnimation_Hook, __this, startPer, endPer, callback, method);
	}

	void MoleMole_EquipOverviewPageContext_PlayLevelUpSuccessShow_Hook(/*MoleMole_EquipOverviewPageContext*/ void* __this, /*Action*/ void* refreshViewAction, MethodInfo* method)
	{
		auto& skipEnhaceAnimation = SkipEnhanceAnimation::GetInstance();
		if (skipEnhaceAnimation.f_Enabled)
			return app::Action_Invoke(refreshViewAction, method);

		CALL_ORIGIN(MoleMole_EquipOverviewPageContext_PlayLevelUpSuccessShow_Hook, __this, refreshViewAction, method);
	}
}