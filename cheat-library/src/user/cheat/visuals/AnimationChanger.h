#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{

	class AnimationChanger : public Feature
	{
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<std::string> f_Animation;
		config::Field<Hotkey> f_ApplyKey;
		config::Field<Hotkey> f_ResetKey;
		config::Field<int> f_Delay;
		config::Field<bool> f_Debug;

		bool onEntityAppear = false;

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;
		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;
		static AnimationChanger& GetInstance();
		void OnGameUpdate();

	private:
		AnimationChanger();
	};
}

