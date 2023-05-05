#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{
	class PaimonFollow : public Feature
	{
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		static PaimonFollow& GetInstance();
		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;
	private:

		void OnGameUpdate();
		PaimonFollow();
	};
}