#include "pch-il2cpp.h"
#include "FallControl.h"

#include <cheat/events.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
    bool FallControl::isFalling = false;

    FallControl::FallControl() : Feature(),
        NFP(f_Enabled, "FallControl", "Fall-Control", false),
        NF(f_Speed, "FallControl", 10.0f)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(FallControl::OnGameUpdate);
        events::MoveSyncEvent += MY_METHOD_HANDLER(FallControl::OnMoveSync);
    }

    const FeatureGUIInfo& cheat::feature::FallControl::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Fall-Control", "Player");
        return info;
    }

    void cheat::feature::FallControl::DrawMain()
    {
        ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Enables fall control"));
        ConfigWidget(_TR("Speed"), f_Speed, 1.0f, 0.0f, 100.0f, _TR("Movement speed when using fall control"));
    }

    bool cheat::feature::FallControl::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void cheat::feature::FallControl::DrawStatus()
    {
        ImGui::Text("%s [%.1f]", _TR("Fall-Control"), f_Speed.value());
    }

    FallControl& cheat::feature::FallControl::GetInstance()
    {
        static FallControl instance;
        return instance;
    }

    // Fall control update function
    // Detects and moves avatar when movement keys are pressed.
    void FallControl::OnGameUpdate()
    {
        if (!f_Enabled->enabled() || !isFalling)
            return;

        auto& manager = game::EntityManager::instance();

        const auto avatarEntity = manager.avatar();
        auto rigidBody = avatarEntity->rigidbody();
        if (rigidBody == nullptr)
            return;

        const auto cameraEntity = game::Entity(reinterpret_cast<app::BaseEntity*>(manager.mainCamera()));
        app::Vector3 direction = {};
        if (Hotkey(ImGuiKey_W).IsPressed())
            direction = direction + cameraEntity.forward();;
        if (Hotkey(ImGuiKey_S).IsPressed())
            direction = direction + cameraEntity.back();;
        if (Hotkey(ImGuiKey_D).IsPressed())
            direction = direction + cameraEntity.right();;
        if (Hotkey(ImGuiKey_A).IsPressed())
            direction = direction + cameraEntity.left();;
        if (IsVectorZero(direction))
            return;
        // Do not change falling velocity when camera relative
        direction.y = 0;

        // Alternative, using set_velocity. Does not work while falling?
        // const float speed = f_Speed.value();
        // const auto currentVelocity = app::Rigidbody_get_velocity(rigidBody, nullptr);
        // const auto desiredvelocity = currentVelocity + direction * speed;
        // LOG_DEBUG("Current velocity: [%.1f,%.1f,%.1f]", currentVelocity.x, currentVelocity.y, currentVelocity.z);
        // LOG_DEBUG("Desired velocity: [%.1f,%.1f,%.1f]\n", desiredvelocity.x, desiredvelocity.y, desiredvelocity.z);
        // app::Rigidbody_set_collisionDetectionMode(rigidBody, app::CollisionDetectionMode__Enum::Continuous, nullptr);
        // app::Rigidbody_set_velocity(rigidBody, desiredvelocity, nullptr);

        const app::Vector3 prevPos = avatarEntity->relativePosition();
        const auto currentVelocity = app::Rigidbody_get_velocity(rigidBody, nullptr);
        const float speed = f_Speed.value();
        const float deltaTime = app::Time_get_deltaTime(nullptr);
        const app::Vector3 newPos = prevPos + (currentVelocity + direction * speed) * deltaTime;
        avatarEntity->setRelativePosition(newPos);
    }

    // Detects when player is falling and enables the FallControl cheat
    void FallControl::OnMoveSync(uint32_t entityId, app::MotionInfo* syncInfo)
    {
        if (!f_Enabled->enabled()) 
        {
            // Edgecase for when you turn off cheat while falling
            isFalling = false;
            return;
        }

        const auto motionState = syncInfo->fields.motionState;
        switch (motionState)
        {
            // These states tell us we are falling
        case app::MotionState__Enum::MotionDrop:
            isFalling = true;
            return;
            
            // State that doesn't tell us anything
        case app::MotionState__Enum::MotionStandby:
        case app::MotionState__Enum::MotionNotify:
            return;

            // We are not falling
        default:
            isFalling = false;
            break;
        }
    }
}
