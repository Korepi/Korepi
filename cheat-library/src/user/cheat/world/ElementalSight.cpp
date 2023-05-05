#include "pch-il2cpp.h"
#include "ElementalSight.h"

#include <helpers.h>

namespace cheat::feature
{
    static void LevelSceneElementViewPlugin_Tick_Hook(app::LevelSceneElementViewPlugin* __this, float inDeltaTime, MethodInfo* method);

    ElementalSight::ElementalSight() : Feature(),
        NFP(f_Enabled, "ElementalSight", "Elemental Sight", false)
    {
        HookManager::install(app::MoleMole_LevelSceneElementViewPlugin_Tick, LevelSceneElementViewPlugin_Tick_Hook);
    }

    const FeatureGUIInfo& ElementalSight::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("World");
        return info;
    }

    void ElementalSight::DrawMain()
    {
        ConfigWidget(_TR("Permanent Elemental Sight"), f_Enabled, _TR("Elemental sight is kept on even when moving.\n"
                     "To turn off, toggle off and use Elemental Sight again."));
    }

    bool ElementalSight::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void ElementalSight::DrawStatus()
    {
        ImGui::Text(_TR("Elemental Sight"));
    }

    ElementalSight& ElementalSight::GetInstance()
    {
        static ElementalSight instance;
        return instance;
    }

    static void LevelSceneElementViewPlugin_Tick_Hook(app::LevelSceneElementViewPlugin* __this, float inDeltaTime, MethodInfo* method)
    {
        ElementalSight& ElementalSight = ElementalSight::GetInstance();
        if (ElementalSight.f_Enabled->enabled())
            __this->fields._triggerElementView = true;
        CALL_ORIGIN(LevelSceneElementViewPlugin_Tick_Hook, __this, inDeltaTime, method);
    }
}

