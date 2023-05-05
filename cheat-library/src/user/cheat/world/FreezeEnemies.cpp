#include "pch-il2cpp.h"
#include "FreezeEnemies.h"

#include <helpers.h>

#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/filters.h>

namespace cheat::feature
{

    FreezeEnemies::FreezeEnemies() : Feature(),
        NFP(f_Enabled, "FreezeEnemies", "Freeze Enemies", false)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(FreezeEnemies::OnGameUpdate);
    }

    const FeatureGUIInfo& FreezeEnemies::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("World");
        return info;
    }

    void FreezeEnemies::DrawMain()
    {
        ConfigWidget(_TR("Freeze Enemies"), f_Enabled, _TR("Freezes all enemies' animation speed."));
    }

    bool FreezeEnemies::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void FreezeEnemies::DrawStatus()
    {
        ImGui::Text(_TR("Freeze Enemies"));
    }

    FreezeEnemies& FreezeEnemies::GetInstance()
    {
        static FreezeEnemies instance;
        return instance;
    }

    // Taiga#5555: There's probably be a better way of implementing this. But for now, this is just what I came up with.
    void FreezeEnemies::OnGameUpdate()
    {
        auto& manager = game::EntityManager::instance();
        static bool change = false;

        for (const auto& monster : manager.entities(game::filters::combined::Monsters))
        {
            auto animator = monster->animator();
            auto rigidBody = monster->rigidbody();
            if (animator == nullptr && rigidBody == nullptr)
                return;

            if (f_Enabled->enabled())
            {
                //auto constraints = app::Rigidbody_get_constraints(rigidBody, nullptr);
                //LOG_DEBUG("%s", magic_enum::enum_name(constraints).data());
                app::Rigidbody_set_constraints(rigidBody, app::RigidbodyConstraints__Enum::FreezeAll, nullptr);
                app::Animator_set_speed(animator, 0.f, nullptr);
                change = false;
            }
            else
            {
                app::Rigidbody_set_constraints(rigidBody, app::RigidbodyConstraints__Enum::FreezeRotation, nullptr);
                if (!change)
                {
                    app::Animator_set_speed(animator, 1.f, nullptr);
                    change = true;
                }
            }
        }
    }
}

