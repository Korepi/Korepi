#include "pch-il2cpp.h"
#include "NoClip.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>
#include <cheat-base/render/renderer.h>

namespace cheat::feature
{
	static void HumanoidMoveFSM_LateTick_Hook(app::HumanoidMoveFSM* __this, float deltaTime, MethodInfo* method);
	app::Vector3 zero;

	NoClip::NoClip() : Feature(),
		NFP(f_Enabled, "NoClip", "No Clip", false),
		NFP(f_NoAnimation, "NoClip", "No Animation", true),
		NF(f_UseCustomKeys, "NoClip", false),
		NF(f_ForwardKey, "NoClip", Hotkey(ImGuiKey_W)),
		NF(f_LeftKey, "NoClip", Hotkey(ImGuiKey_A)),
		NF(f_BackKey, "NoClip", Hotkey(ImGuiKey_S)),
		NF(f_RightKey, "NoClip", Hotkey(ImGuiKey_D)),
		NF(f_AscendKey, "NoClip", Hotkey(ImGuiKey_Space)),
		NF(f_DescendKey, "NoClip", Hotkey(ImGuiKey_ModShift)),
		NF(f_AltSpeedKey, "NoClip", Hotkey(ImGuiKey_ModCtrl)),
		NF(f_Speed, "NoClip", 5.5f),
		NF(f_CameraRelative, "NoClip", true),
		NFP(f_VelocityMode, "NoClip", "Velocity Mode", false),
		NFP(f_FreeflightMode, "NoClip", "Freeflight Mode", false),
		NF(f_AltSpeedEnabled, "NoClip", false),
		NF(f_AltSpeed, "NoClip", 1.0f),
		NFP(f_AntiDragBack, "NoClip", "Drag Back Bypass", false)
	{
		HookManager::install(app::MoleMole_HumanoidMoveFSM_LateTick, HumanoidMoveFSM_LateTick_Hook);

		events::GameUpdateEvent += MY_METHOD_HANDLER(NoClip::OnGameUpdate);
		events::MoveSyncEvent += MY_METHOD_HANDLER(NoClip::OnMoveSync);
	}

	const FeatureGUIInfo& NoClip::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("No-Clip", "Player");
		return info;
	}

	void NoClip::DrawMain()
	{
		ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Enables no-clip (fast speed + no collision).\n" \
			"To move, use WASD, Space (go up), and Shift (go down), or customize your own keys."));

		ConfigWidget(_TR("No Animation"), f_NoAnimation, _TR("Disables player animations."));

		ConfigWidget(_TR("Speed"), f_Speed, 0.1f, 2.0f, 100.0f,
			_TR("No-clip move speed.\n" \
			"Not recommended setting above 5.0."));

		ConfigWidget(_TR("Camera-relative movement"), f_CameraRelative,
			_TR("Move relative to camera view instead of avatar view/direction."));

		ConfigWidget(_TR("Alternate No-clip"), f_AltSpeedEnabled,
			_TR("Allows usage of alternate speed when holding down LeftCtrl key.\n" \
			"Useful if you want to temporarily go faster/slower than the no-clip speed setting."));

		if (f_AltSpeedEnabled) {
			ConfigWidget(_TR("Alt Speed"), f_AltSpeed, 0.1f, 2.0f, 100.0f,
				_TR("Alternate no-clip move speed.\n" \
				"Not recommended setting above 5.0."));

			ConfigWidget(_TR("Velocity mode"), f_VelocityMode, _TR("Use velocity instead of position to move."));
			ConfigWidget(_TR("Freeflight mode"), f_FreeflightMode, _TR("Don't remove collisions"));
		}

		ConfigWidget(_TR("Use Custom Keys"), f_UseCustomKeys, _TR("Enable the Use of Custom HotKeys"));

		if (f_UseCustomKeys) {
			ConfigWidget(_TR("Forward HotKey"), f_ForwardKey, true, _TR("Set Forward Key"));
			ConfigWidget(_TR("Left HotKey"), f_LeftKey, true, _TR("Set Left Key"));
			ConfigWidget(_TR("Back HotKey"), f_BackKey, true, _TR("Set Back Key"));
			ConfigWidget(_TR("Right HotKey"), f_RightKey, true, _TR("Set Right Key"));
			ConfigWidget(_TR("Ascend HotKey"), f_AscendKey, true, _TR("Set Ascend Key"));
			ConfigWidget(_TR("Descend HotKey"), f_DescendKey, true, _TR("Set Descend Key"));
			ConfigWidget(_TR("Alt Speed Key"), f_AltSpeedKey, true, _TR("Set AltSpeed HotKey"));
		}
		ConfigWidget(_TR("DragBackBypass"), f_AntiDragBack,
			_TR("Prevent being dragged back by the server due to excessive speed.\n" \
			"When the server dragback player, in-place teleport."));
	}

	bool NoClip::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}

	void NoClip::DrawStatus()
	{
		ImGui::Text("%s%s[%.01f%s%|%s]",
			_TR("No Clip"),
			f_AltSpeedEnabled ? "+Alt " : " ",
			f_Speed.value(),
			f_AltSpeedEnabled ? fmt::format("|{:.1f}", f_AltSpeed.value()).c_str() : "",
			f_CameraRelative ? "CR" : "PR");
	}

	NoClip& NoClip::GetInstance()
	{
		static NoClip instance;
		return instance;
	}

	// No clip update function.
	// We just disabling collision detect and move avatar when no clip moving keys pressed.
	void NoClip::OnGameUpdate()
	{
		static bool isApplied = false;

		auto& manager = game::EntityManager::instance();

		if (!f_Enabled->enabled() && isApplied)
		{
			auto avatarEntity = manager.avatar();
			auto rigidBody = avatarEntity->rigidbody();
			if (rigidBody == nullptr)
				return;

			app::Rigidbody_set_detectCollisions(rigidBody, true, nullptr);

			isApplied = false;
		}

		if (!f_Enabled->enabled())
			return;

		isApplied = true;

		auto avatarEntity = manager.avatar();
		auto baseMove = avatarEntity->moveComponent();
		if (baseMove == nullptr)
			return;

		if (renderer::IsInputLocked())
			return;

		auto rigidBody = avatarEntity->rigidbody();
		if (rigidBody == nullptr)
			return;

		if (!f_FreeflightMode->enabled())
		{
			app::Rigidbody_set_collisionDetectionMode(rigidBody, app::CollisionDetectionMode__Enum::Continuous, nullptr);
			app::Rigidbody_set_detectCollisions(rigidBody, false, nullptr);
		}

		if (!f_VelocityMode->enabled())
			app::Rigidbody_set_velocity(rigidBody, zero, nullptr);

		auto cameraEntity = game::Entity(reinterpret_cast<app::BaseEntity*>(manager.mainCamera()));
		auto relativeEntity = f_CameraRelative ? &cameraEntity : avatarEntity;

		float speed = f_Speed.value();
		if (f_AltSpeedEnabled ? f_UseCustomKeys ? f_AltSpeedKey.value().IsPressed() : Hotkey(ImGuiKey_ModCtrl).IsPressed() : NULL)
			speed = f_AltSpeed.value();

		app::Vector3 dir = {};

		if (f_UseCustomKeys ? f_ForwardKey.value().IsPressed() : Hotkey(ImGuiKey_W).IsPressed())
			dir = dir + relativeEntity->forward();

		if (f_UseCustomKeys ? f_BackKey.value().IsPressed() : Hotkey(ImGuiKey_S).IsPressed())
			dir = dir + relativeEntity->back();

		if (f_UseCustomKeys ? f_RightKey.value().IsPressed() : Hotkey(ImGuiKey_D).IsPressed())
			dir = dir + relativeEntity->right();

		if (f_UseCustomKeys ? f_LeftKey.value().IsPressed() : Hotkey(ImGuiKey_A).IsPressed())
			dir = dir + relativeEntity->left();

		if (f_UseCustomKeys ? f_AscendKey.value().IsPressed() : Hotkey(ImGuiKey_Space).IsPressed())
			dir = dir + avatarEntity->up();

		if (f_UseCustomKeys ? f_DescendKey.value().IsPressed() : Hotkey(ImGuiKey_ModShift).IsPressed())
			dir = dir + avatarEntity->down();

		app::Vector3 prevPos = avatarEntity->relativePosition();
		if (IsVectorZero(prevPos))
			return;

		float deltaTime = app::Time_get_deltaTime(nullptr);

		app::Vector3 newPos = prevPos + dir * speed * deltaTime;
		if (!f_VelocityMode->enabled())
			avatarEntity->setRelativePosition(newPos);
		else
			app::Rigidbody_set_velocity(rigidBody, dir * speed, nullptr);
	}

	// Fixing player sync packets when no clip
	void NoClip::OnMoveSync(uint32_t entityId, app::MotionInfo* syncInfo)
	{
		static app::Vector3 prevPosition = {};
		static int64_t prevSyncTime = 0;

		if (!f_Enabled->enabled())
		{
			prevSyncTime = 0;
			return;
		}

		auto& manager = game::EntityManager::instance();
		if (manager.avatar()->runtimeID() != entityId)
			return;

		auto avatarEntity = manager.avatar();
		if (avatarEntity == nullptr)
			return;

		auto avatarPosition = avatarEntity->absolutePosition();
		auto currentTime = util::GetCurrentTimeMillisec();
		if (prevSyncTime > 0)
		{
			auto posDiff = avatarPosition - prevPosition;
			auto timeDiff = ((float)(currentTime - prevSyncTime)) / 1000;
			auto velocity = posDiff / timeDiff;

			auto speed = GetVectorMagnitude(velocity);
			if (speed > 0.1)
			{
				syncInfo->fields.motionState = (speed < 2) ? app::MotionState__Enum::MotionWalk : app::MotionState__Enum::MotionRun;

				syncInfo->fields.speed_->fields.x = velocity.x;
				syncInfo->fields.speed_->fields.y = velocity.y;
				syncInfo->fields.speed_->fields.z = velocity.z;
			}

			syncInfo->fields.pos_->fields.x = avatarPosition.x;
			syncInfo->fields.pos_->fields.y = avatarPosition.y;
			syncInfo->fields.pos_->fields.z = avatarPosition.z;
		}

		prevPosition = avatarPosition;
		prevSyncTime = currentTime;
	}

	// Disabling standard motion performing.
	// This disabling any animations, climb, jump, swim and so on.
	// But when it disabled, MoveSync sending our last position, so needs to update position in packet.
	static void HumanoidMoveFSM_LateTick_Hook(app::HumanoidMoveFSM* __this, float deltaTime, MethodInfo* method)
	{
		NoClip& noClip = NoClip::GetInstance();

		if (noClip.f_Enabled->enabled()) 
		{
			if (!noClip.f_NoAnimation->enabled()) 
				__this->fields._layerMaskScene = 2;
			else
				return;
		}

		CALL_ORIGIN(HumanoidMoveFSM_LateTick_Hook, __this, deltaTime, method);
	}
}
