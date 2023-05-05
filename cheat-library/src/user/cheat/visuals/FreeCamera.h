#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{
	class FreeCamera : public Feature
	{
	public:
		config::Field<TranslatedHotkey> f_Enabled;
		config::Field<TranslatedHotkey> f_FreezeAnimation;
		config::Field<TranslatedHotkey> f_SetAvatarInvisible;
		config::Field<bool> f_BlockInput;
		config::Field<bool> f_DamageOverlay;
		config::Field<bool> f_HpOverlay;
		config::Field<float> f_Speed;
		config::Field<float> f_LookSens;
		config::Field<float> f_RollSpeed;
		config::Field<float> f_FOVSpeed;
		config::Field<float> f_FOV;
		config::Field<float> f_MovSmoothing;
		config::Field<float> f_LookSmoothing;
		config::Field<float> f_RollSmoothing;
		config::Field<float> f_FovSmoothing;
		config::Field<Hotkey> f_Forward;
		config::Field<Hotkey> f_Backward;
		config::Field<Hotkey> f_Left;
		config::Field<Hotkey> f_Right;
		config::Field<Hotkey> f_Up;
		config::Field<Hotkey> f_Down;
		config::Field<Hotkey> f_IncFOV;
		config::Field<Hotkey> f_DecFOV;
		config::Field<Hotkey> f_LeftRoll;
		config::Field<Hotkey> f_RightRoll;
		config::Field<Hotkey> f_ResetRoll;

		static FreeCamera& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		bool NeedStatusDraw() const override;
		void DrawStatus() override;

		void OnGameUpdate();

	private:
		FreeCamera();
	};
}

