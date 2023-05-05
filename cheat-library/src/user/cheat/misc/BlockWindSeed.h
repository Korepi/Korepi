#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>
#include <map>

#include <il2cpp-appdata.h>

namespace cheat::feature
{
	class BlockWindSeed : public Feature
	{
	public:
		config::Field<bool> f_Enabled;

		static BlockWindSeed& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

	private:
		BlockWindSeed();
	};
}
