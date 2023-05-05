#include "pch-il2cpp.h"
#include "HideUI.h"

#include <helpers.h>
#include <cheat/events.h>

namespace cheat::feature
{
    app::GameObject* ui_camera{};

    HideUI::HideUI() : Feature(),
		NFP(f_Enabled, "Visuals::HideUI", "Hide UI", false)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(HideUI::OnGameUpdate);
    }

    const FeatureGUIInfo& HideUI::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("Visuals");
        return info;
    }

    void HideUI::DrawMain()
    {
        ConfigWidget(_TR("Hide UI"), f_Enabled, _TR("Hide in-game UI.\nWhen you switch to another scene (e.g. into Domains), turn off and re-enable this feature"));
    }

    bool HideUI::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void HideUI::DrawStatus()
    {
        ImGui::Text(_TR("Hide UI"));
    }

    HideUI& HideUI::GetInstance()
    {
        static HideUI instance;
        return instance;
    }

    void HideUI::OnGameUpdate()
    {
        if (f_Enabled->enabled())
        {
            if (ui_camera == nullptr)
                ui_camera = app::GameObject_Find(string_to_il2cppi("/UICamera"), nullptr);
  
            if (ui_camera->fields._.m_CachedPtr != nullptr)
                app::GameObject_SetActive(ui_camera, false, nullptr);
        }
        else
        {
            if (ui_camera)
            {
                if (ui_camera->fields._.m_CachedPtr != nullptr)
                    app::GameObject_SetActive(ui_camera, true, nullptr);

                ui_camera = nullptr;
            }
        }
    }
}