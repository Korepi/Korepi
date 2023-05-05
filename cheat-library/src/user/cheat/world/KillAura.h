#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature 
{

	class KillAura : public Feature
    {
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<TranslatedHotkey> f_DamageMode;
		config::Field<TranslatedHotkey> f_PercentDamageMode;
		config::Field<TranslatedHotkey> f_InstantDeathMode;
		config::Field<bool> f_OnlyTargeted;
		config::Field<float> f_Range;
		config::Field<int> f_AttackDelay;
		config::Field<int> f_RepeatDelay;
		config::Field<float> f_DamageValue;
		config::Field<int> f_PercentDamageTimes;


		static KillAura& GetInstance();

		void OnGameUpdate();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;
	
	private:
		KillAura();
	};
}

