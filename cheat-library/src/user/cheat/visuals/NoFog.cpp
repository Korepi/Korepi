#include "pch-il2cpp.h"
#include "NoFog.h"

#include <helpers.h>
#include <cheat/events.h>

namespace cheat::feature
{
    static bool _prevEnabledState = false;
    NoFog::NoFog() : Feature(),
        NFP(f_Enabled, "Visuals::NoFog", "No fog", false)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(NoFog::OnGameUpdate);
    }

    const FeatureGUIInfo& NoFog::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("Visuals");
        return info;
    }

    void NoFog::DrawMain()
    {
        ConfigWidget(_TR("No fog"), f_Enabled, _TR("Removes the fog."));
    }

    bool NoFog::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void NoFog::DrawStatus()
    {
        ImGui::Text(_TR("No fog"));
    }

    NoFog& NoFog::GetInstance()
    {
        static NoFog instance;
        return instance;
    }

    void NoFog::OnGameUpdate()
    {
		if (_prevEnabledState != f_Enabled->enabled())
		{
			app::RenderSettings_set_fog(!f_Enabled->enabled(), nullptr);
			_prevEnabledState = f_Enabled->enabled();
		}
    }
}

