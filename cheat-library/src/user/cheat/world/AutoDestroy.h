#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature 
{

	class AutoDestroy : public Feature
    {
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<TranslatedHotkey> f_DestroyOres;
		config::Field<TranslatedHotkey> f_DestroyShields;
		config::Field<TranslatedHotkey> f_DestroyDoodads;
		config::Field<TranslatedHotkey> f_DestroyPlants;
		config::Field<TranslatedHotkey> f_DestroySpecialObjects;
		config::Field<TranslatedHotkey> f_DestroySpecialChests;
		config::Field<float> f_Range;

		static AutoDestroy& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;
	
	private:
		AutoDestroy();
	};
}

