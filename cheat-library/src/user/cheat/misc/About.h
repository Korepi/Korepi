#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature 
{

	class About : public Feature
    {
	public:
		static About& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;
		void OnGameUpdate();
		void ShowInGameScamWarning();
		std::pair<std::wstring, std::wstring> GetScamWarningW();
		bool show = true;
		bool m_IsScamWarningShowed = false;
		int width;
		int height;
		int width_picture;
		int height_picture;
		int timer;
		std::string lang;
		config::Field<bool> f_IsFirstTime;
	private:
		About();
	};
}

