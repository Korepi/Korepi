#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{

	class OpenTeamImmediately : public Feature
	{
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		//MoleMole_InLevelPlayerProfilePageContext*
		void* InLevelPlayerProfilePageContext;

		static OpenTeamImmediately& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

	private:
		OpenTeamImmediately();
	};
}

