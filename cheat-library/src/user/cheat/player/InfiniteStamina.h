#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

#include <il2cpp-appdata.h>

namespace cheat::feature 
{

	class InfiniteStamina : public Feature
    {
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<bool> f_PacketReplacement;

		static InfiniteStamina& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

	private:
		InfiniteStamina();

		bool OnPropertySet(app::PropType__Enum propType);
		void OnMoveSync(uint32_t entityId, app::MotionInfo* syncInfo);

		static void DataItem_HandleNormalProp_Hook(app::DataItem* __this, uint32_t type, int64_t value, app::DataPropOp__Enum state, MethodInfo* method);
		static void VCHumanoidMove_MNKKEGMDFFO_Hook(app::VCHumanoidMove* __this, float JJJEOEHLNGP, MethodInfo* method);
	};
}

