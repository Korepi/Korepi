#pragma once
#include <cheat-base/cheat/Feature.h>

namespace cheat::feature 
{
	class Debug : public Feature
    {
	public:

		static Debug& GetInstance();

		enum class TeleportCondition {
			Closest,
			Farthest
		};

		enum class EntitySortCondition {
			RuntimeID = 0,
			Name,
			Distance
		};

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedInfoDraw() const override;
		void DrawInfo() override;
	
		void DrawExternal() override;
	private:
		Debug();
	};
}

