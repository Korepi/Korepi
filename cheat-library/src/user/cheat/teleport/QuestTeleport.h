#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{
	class QuestTeleport : public Feature
	{
	public:
		config::Field<TranslatedHotkey> f_QuestTP;
		config::Field<Hotkey> f_Key;

		static QuestTeleport& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;
		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

		void QuestPointTeleport();

	private:
		QuestTeleport();
	};
}