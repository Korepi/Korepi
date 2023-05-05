#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{
	class EmotionChanger : public Feature
	{
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<std::string> f_Phonemes;
		config::Field<std::string> f_Emotions;
		config::Field<Hotkey> f_ApplyKey;
		config::Field<Hotkey> f_ResetKey;

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;
		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;
		static EmotionChanger& GetInstance();
		void OnGameUpdate();

	private:
		EmotionChanger();
	};
}