#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{

	class CameraZoom : public Feature
	{
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<TranslatedHotkey> f_SetFov;
		config::Field<float> f_FixedZoom;
		config::Field<float> f_MaxZoom;
		config::Field<float> f_MinZoom;
		config::Field<float> f_ZoomSpeed;
		config::Field<float> f_FovValue;

		static CameraZoom& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

	private:
		CameraZoom();
	};
}

