#include "pch-il2cpp.h"
#include "GameSpeed.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
    GameSpeed::GameSpeed() : Feature(),
        NFP(f_Enabled, "GameSpeed", "Game Speed", false),
        NF(f_Hotkey, "GameSpeed", Hotkey(VK_CAPITAL)),
        NF(f_Speed, "GameSpeed", 5.0f)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(GameSpeed::OnGameUpdate);
    }

    const FeatureGUIInfo& GameSpeed::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Game Speed", "World");
        return info;
    }

    void GameSpeed::DrawMain()
    {
        ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Speeds up game with hotkey"));
		ConfigWidget(_TR("Hotkey"), f_Hotkey);
		ConfigWidget(_TR("Multiplier"), f_Speed, 1.0f, 0.0f, 20.0f, _TR("Set GameSpeed Multiplier\n" \
			"Do NOT use this in the Open World, only use in menus/etc, VERY DANGEROUS!"));
    }

    bool GameSpeed::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void GameSpeed::DrawStatus()
    {
        ImGui::Text(_TR("Game Speed"));
    }

    GameSpeed& GameSpeed::GetInstance()
    {
        static GameSpeed instance;
        return instance;
    }

    void GameSpeed::OnGameUpdate()
    {
        static bool isSpeed = false;
        float currentSpeed = app::Time_get_timeScale(nullptr);
        if (f_Enabled->enabled())
        {
			SAFE_BEGIN();
            if (f_Hotkey.value().IsPressed())
            {
                if (currentSpeed == 1.0f)
                {
                    app::Time_set_timeScale(f_Speed, nullptr);
                    isSpeed = true;
                }
            }

            if (!f_Hotkey.value().IsPressed() && isSpeed)
            {
                if (currentSpeed != 1.0f)
                {
                    app::Time_set_timeScale(1.0f, nullptr);
                    isSpeed = false;
                }
            }
			SAFE_EEND();
        }
        else
        {
            // Aditional check if user is still pressing key and they decide to disable the feature
            if (isSpeed)
            {
                if (currentSpeed != 1.0f)
                {
                    app::Time_set_timeScale(1.0f, nullptr);
                    isSpeed = false;
                }
            }
        }
    }
}
