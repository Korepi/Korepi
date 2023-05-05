#include "pch-il2cpp.h"
#include "FreeCamera.h"

#include <math.h>
#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
	app::GameObject* freeCam = nullptr;
	app::GameObject* mainCam = nullptr;
	app::Object_1* freeCamObj = nullptr;
	app::Object_1* mainCamObj = nullptr;
	app::GameObject* damageOverlay = nullptr;
	app::GameObject* hpOverlay = nullptr;
	app::Transform* freeCam_Transform;
	app::Component_1* freeCam_Camera;
	app::Component_1* mainCam_Camera;
	app::Vector3 targetPosition;
	app::Vector3 smoothPosition;
	float smoothFOV = 0;
	double focalLength = 0;
	bool isEnabled = false;

	FreeCamera::FreeCamera() : Feature(),
		NFP(f_Enabled, "Visuals::FreeCamera", "Free Camera", false),
		NFP(f_FreezeAnimation, "Visuals::FreeCamera", "Freeze animation", false),
		NFP(f_SetAvatarInvisible, "Visuals::FreeCamera", "Set avatar invisible", false),
		NF(f_BlockInput, "Visuals::FreeCamera", false),
		NF(f_DamageOverlay, "Visuals::FreeCamera", false),
		NF(f_HpOverlay, "Visuals::FreeCamera", false),
		NF(f_Speed, "Visuals::FreeCamera", 1.0f),
		NF(f_LookSens, "Visuals::FreeCamera", 1.0f),
		NF(f_RollSpeed, "Visuals::FreeCamera", 1.0f),
		NF(f_FOVSpeed, "Visuals::FreeCamera", 0.1f),
		NF(f_FOV, "Visuals::FreeCamera", 45.0f),
		NF(f_MovSmoothing, "Visuals::FreeCamera", 1.0f),
		NF(f_LookSmoothing, "Visuals::FreeCamera", 1.0f),
		NF(f_RollSmoothing, "Visuals::FreeCamera", 1.0f),
		NF(f_FovSmoothing, "Visuals::FreeCamera", 1.0f),
		NF(f_Forward, "Visuals::FreeCamera", Hotkey('W')),
		NF(f_Backward, "Visuals::FreeCamera", Hotkey('S')),
		NF(f_Left, "Visuals::FreeCamera", Hotkey('A')),
		NF(f_Right, "Visuals::FreeCamera", Hotkey('D')),
		NF(f_Up, "Visuals::FreeCamera", Hotkey(VK_SPACE)),
		NF(f_Down, "Visuals::FreeCamera", Hotkey(VK_LCONTROL)),
		NF(f_LeftRoll, "Visuals::FreeCamera", Hotkey('Z')),
		NF(f_RightRoll, "Visuals::FreeCamera", Hotkey('X')),
		NF(f_ResetRoll, "Visuals::FreeCamera", Hotkey('C')),
		NF(f_IncFOV, "Visuals::FreeCamera", Hotkey('3')),
		NF(f_DecFOV, "Visuals::FreeCamera", Hotkey('1'))
	{
		events::GameUpdateEvent += MY_METHOD_HANDLER(FreeCamera::OnGameUpdate);
	}

	const FeatureGUIInfo& FreeCamera::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Free Camera", "Visuals");
		return info;
	}

	void FreeCamera::DrawMain()
	{
		ConfigWidget(_TR("Enable"), f_Enabled);
		ConfigWidget(_TR("Freeze Character Animation"), f_FreezeAnimation, _TR("Freezes the active character's animation."));
		ConfigWidget(_TR("Make Character invisible"), f_SetAvatarInvisible, _TR("Hide Character machine effects."));
		ConfigWidget(_TR("Block User Input"), f_BlockInput, _TR("If enabled, any input will be blocked."));
		if (f_Enabled->enabled())
		{
			ConfigWidget(_TR("Toggle Damage Overlay"), f_DamageOverlay, _TR("Remove damage output overlay"));
			if (ImGui::Button(_TR("Remove HP")))
			{
				f_HpOverlay = true;
			}
		}

		if (ImGui::BeginTable("FreeCameraDrawTable", 1, ImGuiTableFlags_NoBordersInBody))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::BeginGroupPanel(_TR("Settings"));
			{
				ConfigWidget(_TR("Movement Speed"), f_Speed, 0.01f, 0.01f, 1000.0f);
				ConfigWidget(_TR("Look Sensitivity"), f_LookSens, 0.01f, 0.01f, 100.0f);
				ConfigWidget(_TR("Roll Speed"), f_RollSpeed, 0.01f, 0.01f, 100.0f);
				ConfigWidget(_TR("FOV Speed"), f_FOVSpeed, 0.01f, 0.01f, 100.0f);
				ConfigWidget(_TR("Field of View"), f_FOV, 0.1f, 0.01f, 200.0f, _TR("Changes Vertical FoV. Horizontal FoV depends on the viewport's aspect ratio"));
				if (ImGui::Button(_TR("Convert FoV to 35mm FF focal length")))
					focalLength = 24 / (2 * tan((f_FOV * 3.14159265) / (2 * 180))); // FocalLength = (vertical) sensor size / 2 * tan( 2*(vertical) FoV * Pi / 180)  Remember to convert degree to radian.  
				ImGui::Text("%s: %f", _TR("Focal length"), focalLength);
				ImGui::Spacing();
				ConfigWidget(_TR("Movement Smoothing"), f_MovSmoothing, 0.01f, 0.001f, 1.0f, _TR("Lower = Smoother"));
				ConfigWidget(_TR("Look Smoothing"), f_LookSmoothing, 0.01f, 0.001f, 1.0f, _TR("Lower = Smoother"));
				ConfigWidget(_TR("Roll Smoothing"), f_RollSmoothing, 0.01f, 0.001f, 1.0f, _TR("Lower = Smoother"));
				ConfigWidget(_TR("FOV Smoothing"), f_FovSmoothing, 0.01f, 0.001f, 1.0f, _TR("Lower = Smoother"));
			}
			ImGui::EndGroupPanel();

			ImGui::BeginGroupPanel(_TR("Hotkeys"));
			{
				ConfigWidget(_TR("Forward"), f_Forward, true);
				ConfigWidget(_TR("Backward"), f_Backward, true);
				ConfigWidget(_TR("Left"), f_Left, true);
				ConfigWidget(_TR("Right"), f_Right, true);
				ConfigWidget(_TR("Up"), f_Up, true);
				ConfigWidget(_TR("Down"), f_Down, true);
				ConfigWidget(_TR("Roll Left"), f_LeftRoll, true);
				ConfigWidget(_TR("Roll Right"), f_RightRoll, true);
				ConfigWidget(_TR("Reset Roll"), f_ResetRoll, true);
				ConfigWidget(_TR("Increase FOV"), f_IncFOV, true);
				ConfigWidget(_TR("Decrease FOV"), f_DecFOV, true);
			}
			ImGui::EndGroupPanel();
			ImGui::EndTable();
		}
	}

	bool FreeCamera::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}

	void FreeCamera::DrawStatus()
	{
		ImGui::Text(_TR("Free Camera"));
	}

	FreeCamera& FreeCamera::GetInstance()
	{
		static FreeCamera instance;
		return instance;
	}

	class CameraRotation
	{
	public:
		float pitch, yaw, roll;

		void InitializeFromTransform(app::Transform* t)
		{
			auto t_eulerAngles = app::Transform_get_eulerAngles(t, nullptr);
			pitch = t_eulerAngles.x;
			yaw = t_eulerAngles.y;
			roll = t_eulerAngles.z;
		}

		void LerpTowards(CameraRotation target, float lookRotationLerpPct, float rollRotationLerpPct)
		{
			yaw = app::Mathf_Lerp(yaw, target.yaw, lookRotationLerpPct, nullptr);
			pitch = app::Mathf_Lerp(pitch, target.pitch, lookRotationLerpPct, nullptr);
			roll = app::Mathf_Lerp(roll, target.roll, rollRotationLerpPct, nullptr);
		}

		void UpdateTransform(app::Transform* t)
		{
			app::Transform_set_eulerAngles(t, app::Vector3{ pitch, yaw, roll }, nullptr);
		}
	};

	auto targetRotation = CameraRotation();
	auto currentRotation = CameraRotation();

	void EnableFreeCam()
	{
		auto& settings = FreeCamera::GetInstance();
		freeCam = reinterpret_cast<app::GameObject*>(freeCamObj);

		freeCam_Transform = app::GameObject_get_transform(freeCam, nullptr);
		auto freeCam_Transform_position = app::Transform_get_position(freeCam_Transform, nullptr);

		freeCam_Camera = app::GameObject_GetComponentByName(freeCam, string_to_il2cppi("Camera"), nullptr);
		mainCam_Camera = app::GameObject_GetComponentByName(mainCam, string_to_il2cppi("Camera"), nullptr);

		if (isEnabled == false)
		{
			targetRotation.InitializeFromTransform(freeCam_Transform);
			currentRotation.InitializeFromTransform(freeCam_Transform);
			app::Camera_CopyFrom(reinterpret_cast<app::Camera*>(freeCam_Camera), reinterpret_cast<app::Camera*>(mainCam_Camera), nullptr);

			targetPosition = freeCam_Transform_position;
			isEnabled = true;
		}

		app::GameObject_set_active(mainCam, false, nullptr);
		app::GameObject_set_active(freeCam, true, nullptr);

		// MOVEMENT
		if (settings.f_Forward.value().IsPressed())
			targetPosition = targetPosition + app::Transform_get_forward(freeCam_Transform, nullptr) * settings.f_Speed;
		if (settings.f_Backward.value().IsPressed())
			targetPosition = targetPosition - app::Transform_get_forward(freeCam_Transform, nullptr) * settings.f_Speed;
		if (settings.f_Right.value().IsPressed())
			targetPosition = targetPosition + app::Transform_get_right(freeCam_Transform, nullptr) * settings.f_Speed;
		if (settings.f_Left.value().IsPressed())
			targetPosition = targetPosition - app::Transform_get_right(freeCam_Transform, nullptr) * settings.f_Speed;

		if (settings.f_LeftRoll.value().IsPressed())
			targetRotation.roll += settings.f_RollSpeed;
		if (settings.f_RightRoll.value().IsPressed())
			targetRotation.roll -= settings.f_RollSpeed;
		if (settings.f_ResetRoll.value().IsPressed())
			targetRotation.roll = 0.0f;

		if (settings.f_Up.value().IsPressed())
			targetPosition = targetPosition + app::Transform_get_up(freeCam_Transform, nullptr) * settings.f_Speed;
		if (settings.f_Down.value().IsPressed())
			targetPosition = targetPosition - app::Transform_get_up(freeCam_Transform, nullptr) * settings.f_Speed;

		if (settings.f_DecFOV.value().IsPressed())
			settings.f_FOV -= settings.f_FOVSpeed;
		if (settings.f_IncFOV.value().IsPressed())
			settings.f_FOV += settings.f_FOVSpeed;

		// Update the target rotation based on mouse input
		auto mouseX = app::Input_GetAxis(string_to_il2cppi("Mouse X"), nullptr);
		auto mouseY = app::Input_GetAxis(string_to_il2cppi("Mouse Y"), nullptr);
		auto mouseInput = app::Vector2{ mouseX, mouseY * -1.0f };
		targetRotation.yaw += mouseInput.x * settings.f_LookSens;
		targetRotation.pitch += mouseInput.y * settings.f_LookSens;

		// Commit the rotation changes to the transform
		currentRotation.UpdateTransform(freeCam_Transform);

		smoothPosition = app::Vector3_Lerp(freeCam_Transform_position, targetPosition, settings.f_MovSmoothing, nullptr);
		app::Transform_set_position(freeCam_Transform, smoothPosition, nullptr);
		smoothFOV = app::Mathf_Lerp(app::Camera_get_fieldOfView(reinterpret_cast<app::Camera*>(freeCam_Camera), nullptr), settings.f_FOV, settings.f_FovSmoothing, nullptr);
		app::Camera_set_fieldOfView(reinterpret_cast<app::Camera*>(freeCam_Camera), smoothFOV, nullptr);
		currentRotation.LerpTowards(targetRotation, settings.f_LookSmoothing, settings.f_RollSmoothing);
	}

	void DisableFreeCam()
	{
		if (!isEnabled)
			return;

		if (mainCam)
		{
			app::GameObject_set_active(mainCam, true, nullptr);
			mainCam = nullptr;
		}
		if (freeCamObj)
		{
			app::Object_1_Destroy_1(freeCamObj, nullptr);
			freeCamObj = nullptr;
		}
		isEnabled = false;
	}

	void FreeCamera::OnGameUpdate()
	{
		auto uiManager = GET_SINGLETON(MoleMole_UIManager);
		if (uiManager == nullptr)
			return;

		static bool isBlock = false;

		if (f_Enabled->enabled())
		{
			if (mainCam == nullptr)
				mainCam = app::GameObject_Find(string_to_il2cppi("/EntityRoot/MainCamera(Clone)"), nullptr);
			if (freeCamObj == nullptr && mainCam)
			{
				freeCamObj = app::Object_1_Instantiate_2(reinterpret_cast<app::Object_1*>(mainCam), nullptr);

				auto mainCamTransform = app::GameObject_get_transform(mainCam, nullptr);
				auto mainCamPos = app::Transform_get_position(mainCamTransform, nullptr);
				auto freeCamObjTransform = app::GameObject_get_transform(reinterpret_cast<app::GameObject*>(freeCamObj), nullptr);
				app::Transform_set_position(freeCamObjTransform, mainCamPos, nullptr);

				auto CinemachineBrain = app::GameObject_GetComponentByName(reinterpret_cast<app::GameObject*>(freeCamObj), string_to_il2cppi("CinemachineBrain"), nullptr);
				auto CinemachineExternalCamera = app::GameObject_GetComponentByName(reinterpret_cast<app::GameObject*>(freeCamObj), string_to_il2cppi("CinemachineExternalCamera"), nullptr);
				app::Object_1_Destroy_1(reinterpret_cast<app::Object_1*>(CinemachineBrain), nullptr);
				app::Object_1_Destroy_1(reinterpret_cast<app::Object_1*>(CinemachineExternalCamera), nullptr);

				app::GameObject_set_active(mainCam, false, nullptr);
				app::GameObject_set_active(mainCam, true, nullptr);
				app::GameObject_set_active(reinterpret_cast<app::GameObject*>(freeCamObj), false, nullptr);
			}
			if (freeCamObj)
				EnableFreeCam();

			if (damageOverlay == nullptr)
				damageOverlay = app::GameObject_Find(string_to_il2cppi("/Canvas/Pages/InLevelMainPage/GrpMainPage/ParticleDamageTextContainer"), nullptr);
			else
				app::GameObject_SetActive(damageOverlay, !f_DamageOverlay, nullptr);

			if (f_HpOverlay)  //Fixed an issue where HpOverlay could not be removed properly.
			{
				Sleep(200);
				f_HpOverlay = false;
				hpOverlay = app::GameObject_Find(string_to_il2cppi("AvatarBoardCanvasV2(Clone)"), nullptr);
				while (hpOverlay != nullptr)
				{
					app::GameObject_SetActive(hpOverlay, false, nullptr);
					hpOverlay = app::GameObject_Find(string_to_il2cppi("AvatarBoardCanvasV2(Clone)"), nullptr);
				}
			}

			if (f_BlockInput) {
				if (!isBlock) {
					app::MoleMole_UIManager_EnableInput(uiManager, false, false, false, nullptr);
					isBlock = true;
				}
			} else {
				if (isBlock) {
					app::MoleMole_UIManager_EnableInput(uiManager, true, false, false, nullptr);
					isBlock = false;
				}
			}
		}
		else
		{
			DisableFreeCam();
			damageOverlay = nullptr;
			hpOverlay = nullptr;

			if (isBlock) {
				app::MoleMole_UIManager_EnableInput(uiManager, true, false, false, nullptr);
				isBlock = false;
			}
		}

		// Taiga#5555: There's probably be a better way of implementing this. But for now, this is just what I came up with.
		auto& manager = game::EntityManager::instance();
		auto animator = manager.avatar()->animator();
		auto rigidBody = manager.avatar()->rigidbody();
		if (animator == nullptr && rigidBody == nullptr)
			return;

		static bool changed = false;
		static bool isVisible = false;

		if (f_FreezeAnimation->enabled())
		{
			//auto constraints = app::Rigidbody_get_constraints(rigidBody, nullptr);
			//LOG_DEBUG("%s", magic_enum::enum_name(constraints).data());
			app::Rigidbody_set_constraints(rigidBody, app::RigidbodyConstraints__Enum::FreezePosition, nullptr);
			app::Animator_set_speed(animator, 0.f, nullptr);
			changed = false;
		}
		else
		{
			app::Rigidbody_set_constraints(rigidBody, app::RigidbodyConstraints__Enum::FreezeRotation, nullptr);
			if (!changed)
			{
				app::Animator_set_speed(animator, 1.f, nullptr);
				changed = true;
			}
		}
		
		if (f_SetAvatarInvisible->enabled())
		{
			app::Miscs_SetUILocalAvatarVisible(false, nullptr);
			isVisible = false;
		}	
		else
		{
			if (!isVisible)
			{
				app::Miscs_SetUILocalAvatarVisible(true, nullptr);
				isVisible = true;
			}
		}		
	}
}
