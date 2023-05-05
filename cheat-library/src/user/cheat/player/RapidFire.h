#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

#include <il2cpp-appdata.h>

namespace cheat::feature 
{
	class RapidFire : public Feature
    {
	public:
		
		enum class ElementType
		{
			None,
			Pyro,
			Hydro,
			Dendro,
			Electro,
			Cryo,
			Frozen,
			Anemo,
			Geo,
			AntiFire,
			VehicleMuteIce,
			Unknown,
		};
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<TranslatedHotkey> f_MultiHit;
		config::Field<int> f_Multiplier;
		config::Field<bool> f_OnePunch;
		config::Field<TranslatedHotkey> f_Randomize;
		config::Field<int> f_minMultiplier;
		config::Field<int> f_maxMultiplier;
		config::Field<TranslatedHotkey> f_MultiTarget;
		config::Field<float> f_MultiTargetRadius;
		config::Field<TranslatedHotkey> f_MultiAnimation;
		config::Field<int> f_AnimationMultiplier;
		config::Field<float> f_AnimationState;
		config::Field<TranslatedHotkey> f_AttackSpeed;
		config::Field<float> f_SpeedMultiplier;
		config::Field<TranslatedHotkey> f_CustomElement;
		config::Field<config::Enum<ElementType>> f_ElementType;
		config::Field<bool> f_isRandType;
		config::Field<bool> f_HeadShot;
		int animationCounter;

		static RapidFire& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;
	
		int GetAttackCount(app::LCBaseCombat* combat, uint32_t targetID, app::AttackResult* attackResult);
	private:
		RapidFire();
		int CalcCountToKill(float attackDamage, uint32_t targetID);
	};
}


