#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature 
{

	class AutoTreeFarm : public Feature
    {
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<int> f_AttackDelay;
		config::Field<int> f_RepeatDelay;
		config::Field<int> f_AttackPerTree;
		config::Field<float> f_Range;

		static AutoTreeFarm& GetInstance();

		void OnGameUpdate();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;
	
	private:
		AutoTreeFarm();
	};
}

