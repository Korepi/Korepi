#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature 
{
	class NoCD : public Feature
    {
	public:
		config::Field<TranslatedHotkey> f_AbilityReduce;
		config::Field<float> f_TimerReduce;

		config::Field<TranslatedHotkey> f_UtimateMaxEnergy;
		config::Field<TranslatedHotkey> f_Sprint;
		config::Field<TranslatedHotkey> f_InstantBow;

		static NoCD& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;
	
	private:
		NoCD();
	};
}

