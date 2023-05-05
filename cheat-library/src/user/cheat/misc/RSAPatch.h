#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{
	class RSAPatch : public Feature
	{
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<config::Toggle<std::string>> f_PublicKey;
		config::Field<config::Toggle<std::string>> f_PrivateKey;

		static RSAPatch &GetInstance();

		const FeatureGUIInfo &GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

	private:
		RSAPatch();
	};
}