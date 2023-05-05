#include "pch-il2cpp.h"
#include "FlycloakModifier.h"

#include <cheat/game/filters.h>
#include <cheat/events.h>

namespace cheat::feature
{

    FlycloakModifier::FlycloakModifier() : Feature(),
        NFP(f_Enabled, "FlycloakModifier", "Flycloak Modifier", false),
        NF(f_FlycloakType, "FlycloakModifer", FlycloakModifier::FlycloakType::FirstFlight)
    {
        events::EntityAppearEvent += MY_METHOD_HANDLER(FlycloakModifier::OnEntityAppear);
    }

    FlycloakModifier& FlycloakModifier::GetInstance()
    {
        static FlycloakModifier Instance;
        return Instance;
    }

    const FeatureGUIInfo& FlycloakModifier::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Flycloak Modifier", "Visuals");
        return info;
    }

    void FlycloakModifier::DrawMain()
    {
        ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Change The Wings. Change character to apply."));
        ConfigWidget(_TR("Flycloak Type"), f_FlycloakType, _TR("Select The Type Of Wings."));
    }

    bool FlycloakModifier::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void FlycloakModifier::DrawStatus()
    {
        ImGui::Text(_TR("Flycloak Modifier"));
        ImGui::Text("%s", Translator::RuntimeTranslate(magic_enum::enum_name(f_FlycloakType.value().value()).data()).c_str());
    }

    uint32_t FlycloakModifier::GetFlycloakType()
    {
        return flycloakType.at(f_FlycloakType.value());
    }

    void FlycloakModifier::OnEntityAppear(app::Proto_SceneEntityAppearNotify* notify)
    {
        if (!f_Enabled->enabled())
            return;
        auto entityList = notify->fields.entityList_;
        if (entityList == nullptr)
            return;
        for (int i = 0; i < entityList->fields.values->fields._size; i++)
        {
            auto& itemFields = entityList->fields.values->fields._items->vector[i]->fields;
            if (itemFields.entityCase_ == app::Proto_SceneEntityInfo_Proto_SceneEntityInfo_EntityOneofCase__Enum::Avatar)
            {
                auto avatar = CastTo<app::Proto_SceneAvatarInfo>(itemFields.entity_, *app::Proto_SceneAvatarInfo__TypeInfo);
                if (avatar != nullptr)
                    avatar->fields.wearingFlycloakId_ = GetFlycloakType();
            }
        }
    }
}