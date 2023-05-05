#include "pch-il2cpp.h"
#include "ShowChestIndicator.h"

#include <helpers.h>
#include <cheat/events.h>

namespace cheat::feature
{
    static bool IndicatorPlugin_DoCheck(app::LCIndicatorPlugin* __this, MethodInfo* method);

    ChestIndicator::ChestIndicator() : Feature(),
        NFP(f_Enabled, "Visuals::ChestIndicator", "Chest Indicator", false)
    {
        HookManager::install(app::MoleMole_LCIndicatorPlugin_DoCheck, IndicatorPlugin_DoCheck);
    }

    const FeatureGUIInfo& ChestIndicator::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("Visuals");
        return info;
    }

    void ChestIndicator::DrawMain()
    {
        ConfigWidget(_TR("Show Chest Indicator"), f_Enabled, _TR("Show chests, game mechanics."));
    }

    bool ChestIndicator::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void ChestIndicator::DrawStatus()
    {
        ImGui::Text(_TR("Chest Indicator"));
    }

    ChestIndicator& ChestIndicator::GetInstance()
    {
        static ChestIndicator instance;
        return instance;
    }

    static bool IndicatorPlugin_DoCheck(app::LCIndicatorPlugin* __this, MethodInfo* method) {
        ChestIndicator& chestIndicator = ChestIndicator::GetInstance();
        if (chestIndicator.f_Enabled->enabled())
        {
            if (__this->fields._dataItem != nullptr)
            {
                // Base Chest not found, try improve
                app::MoleMole_LCIndicatorPlugin_ShowIcon(__this, nullptr);
            }
        }
        return CALL_ORIGIN(IndicatorPlugin_DoCheck, __this, method);
    }
}