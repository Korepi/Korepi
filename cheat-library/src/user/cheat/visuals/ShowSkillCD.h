#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{

	class ShowSkillCD : public Feature
	{
	public:
		config::Field<bool> f_Enabled;

		static ShowSkillCD& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

	private:
		ShowSkillCD();

		struct SkillData
		{
			uint32_t id;
			float cd;
		};

		std::unordered_map<uint32_t, SkillData> m_skillDataMap;
		std::unordered_map<uint64_t, app::MonoTeamBtn*> m_teamBtnMap;

		static void SetShowKey(void* key, bool show);
		static void UpdateSkillMap(app::LCAvatarCombat* lcCombat, app::LCAvatarCombat_LCAvatarCombat_SkillInfo* skillInfo, float cd);

		void OnGameUpdate();

		static void MoleMole_LCAvatarCombat_SetSkillIndex_Hook(app::LCAvatarCombat* __this, app::LCAvatarCombat_LCAvatarCombat_SkillInfo* skillInfo, int32_t index, int32_t priority, MethodInfo* method);
		static void MoleMole_LCAvatarCombat_CheckCDTimer_Hook(app::LCAvatarCombat* __this, app::LCAvatarCombat_LCAvatarCombat_SkillInfo* info, MethodInfo* method);
		static void MonoTeamBtn_SetupView_Hook(app::MonoTeamBtn* __this, uint64_t guid, MethodInfo* method);
	};
}