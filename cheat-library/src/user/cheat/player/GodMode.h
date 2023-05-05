#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature 
{

	class GodMode : public Feature
    {
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<bool> f_Conditional;
		config::Field<float> f_MinHealth;
		config::Field<float> f_MissingRate;

		static GodMode& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		bool NeedStatusDraw() const override;
		void DrawStatus() override;
	
	private:

		bool NeedBlockHanlerModifierThinkTimeUp(app::Object* arg);
		static bool MoleMole_ActorAbilityPlugin_HanlderModifierThinkTimerUp_Hook(app::ActorAbilityPlugin* __this, float delay, app::Object* arg, MethodInfo* method);
		
		//static void LCBaseCombat_FireBeingHitEvent_Hook(app::LCBaseCombat* __this, uint32_t attackeeRuntimeID, app::AttackResult* attackResult, MethodInfo* method);
		static void VCHumanoidMove_NotifyLandVelocity_Hook(app::VCHumanoidMove* __this, app::Vector3 velocity, float reachMaxDownVelocityTime, MethodInfo* method);
		static bool Miscs_CheckTargetAttackable_Hook(app::BaseEntity* attacker, app::BaseEntity* target, MethodInfo* method);

		GodMode();
	};
}

