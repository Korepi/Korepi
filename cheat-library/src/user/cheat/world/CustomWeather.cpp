#include "pch-il2cpp.h"
#include "CustomWeather.h"
#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>
#include <cheat/game/filters.h>

namespace cheat::feature
{
    static std::map<CustomWeather::WeatherType, std::string> weather
    {
        { CustomWeather::WeatherType::ClearSky, "Data/Environment/Weather/BigWorld/Weather_ClearSky" },
        { CustomWeather::WeatherType::Cloudy, "Data/Environment/Weather/BigWorld/Weather_Cloudy" },
        { CustomWeather::WeatherType::Foggy, "Data/Environment/Weather/BigWorld/Weather_Foggy" },
        { CustomWeather::WeatherType::Storm, "Data/Environment/Weather/BigWorld/Weather_Storm" },
        { CustomWeather::WeatherType::RainHeavy, "Data/Environment/Weather/BigWorld/Weather_Dq_Tabeisha_Rain_Heavy" },
        { CustomWeather::WeatherType::FountainRain, "Data/Environment/Weather/BigWorld/Weather_LY_Fountain_Rain" },
        { CustomWeather::WeatherType::SnowLight, "Data/Environment/Weather/BigWorld/Weather_Snowmountain_Snow_Light" },
        { CustomWeather::WeatherType::EastCoast, "Data/Environment/Weather/BigWorld/Weather_Snowmountain_EastCoast" },
    };

    CustomWeather::CustomWeather() : Feature(),
        NFP(f_Enabled, "CustomWeather", "Custom weather", false),
        NFP(f_Lightning, "CustomWeather", "Lighting", false),
        NF(f_WeatherType, "CustomWeather", CustomWeather::WeatherType::ClearSky)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(CustomWeather::OnGameUpdate);
    }

    const FeatureGUIInfo& CustomWeather::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Custom Weather", "Visuals");
        return info;
    }

    void CustomWeather::DrawMain()
    {
        ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Custom Weather."));
        if (f_Enabled->enabled())
            ConfigWidget(_TR("Weather type"), f_WeatherType, _TR("Select weather type."));
        ConfigWidget(_TR("Lightning"), f_Lightning, _TR("Lightning target enemy, works with RainHeavy weather."));
    }

    bool CustomWeather::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void CustomWeather::DrawStatus()
    {
        ImGui::Text(_TR("Custom Weather"));
        if (f_Lightning->enabled())
            ImGui::Text(_TR("Lightning"));
    }

    CustomWeather& CustomWeather::GetInstance()
    {
        static CustomWeather instance;
        return instance;
    }

    void CustomWeather::OnGameUpdate()
    {
        if (!f_Enabled->enabled())
            return;

        UPDATE_DELAY(100);

        auto Enviro = app::EnviroSky_get_Instance(nullptr);
        if (Enviro != nullptr)
        {
            app::EnviroSky_ChangeWeather(Enviro, string_to_il2cppi(weather.at(f_WeatherType.value())), 1, 1, nullptr);

            if (f_Lightning->enabled() && f_WeatherType.value() == CustomWeather::WeatherType::RainHeavy)
            {
                auto& manager = game::EntityManager::instance();

                for (auto& Monsters : manager.entities(game::filters::combined::Monsters))
                {
                    if (manager.avatar()->distance(Monsters) >= 30)
                        continue;

                    for (auto& entity : manager.entities(game::filters::combined::Lightning))
                    {
                        entity->setRelativePosition(Monsters->relativePosition());
                    }
                }
            }
        }
    }
}