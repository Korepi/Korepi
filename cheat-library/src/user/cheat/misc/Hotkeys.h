#pragma once
#include <cheat-base/cheat/Feature.h>

namespace cheat::feature 
{

	class Hotkeys : public Feature
    {
	public:
		static Hotkeys& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;
	
	private:
		Hotkeys();
	};
}

