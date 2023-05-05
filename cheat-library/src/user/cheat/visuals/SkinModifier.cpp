#include "pch-il2cpp.h"
#include "SkinModifier.h"

#include <cheat/game/filters.h>
#include <cheat/events.h>

namespace cheat::feature
{

    SkinModifier::SkinModifier() : Feature(),
        NFP(f_Enabled, "SkinModifier", "Skin Modifier", false)
    {
        InitSkins();
        events::EntityAppearEvent += MY_METHOD_HANDLER(SkinModifier::OnEntityAppear);
    }

    SkinModifier& SkinModifier::GetInstance()
    {
        static SkinModifier Instance;
        return Instance;
    }

    const FeatureGUIInfo& SkinModifier::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Skin Modifier", "Visuals");
        return info;
    }

    bool SkinModifier::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void SkinModifier::DrawStatus()
    {
        ImGui::Text(_TR("Skin Modifier"));
    }

    void SkinModifier::DrawMain()
    {
        ConfigWidget(_TR("Enabled"), f_Enabled, _TR("How to use: \n1.make sure you enable.\n2.switch to the character that needs to change skin\n3.open team list,confirm the skin is changed.\n4.change to another team and then change back.\nIf more than one skin for the same character is selected, only first will apply."));
        ImGui::PushID("Skins");
        bool checked = std::all_of(m_Skins.begin(), m_Skins.end(), [](const std::pair<config::Field<bool>, Skins>& skin) { return skin.first; });
        bool changed = false;
        if (ImGui::BeginSelectableGroupPanel(Translator::RuntimeTranslate("Skins").c_str(), checked, changed, true))
        {
            for (auto& skin : m_Skins)
            {
                ImGui::PushID(&skin);
                ConfigWidget(Translator::RuntimeTranslate((magic_enum::enum_name(skin.second).data())).c_str(), skin.first);
                ImGui::PopID();
            }
        }
        ImGui::EndSelectableGroupPanel();

        if (changed)
        {
            for (const auto& info : m_Skins)
            {
                info.first.value() = checked;
                info.first.FireChanged();
            }
        }
        ImGui::PopID();
    }

    void SkinModifier::InitSkins()
    {
        for (auto& [_, skins] : m_AllSkins)
        {
            for (auto& [skin, _] : skins)
            {
                m_Skins.push_back({ config::CreateField<bool>(magic_enum::enum_name(skin).data(), "SkinModifier", false, false), skin });
            }
        }
    }

    uint32_t SkinModifier::GetSkinId(uint32_t avaterId)
    {
        auto skins = m_AllSkins.find(avaterId);
        if (skins == m_AllSkins.end())
            return 0;
        for (auto& [skin, skinId] : skins->second)
        {
            if (std::any_of(m_Skins.begin(), m_Skins.end(), [&](std::pair<config::Field<bool>, Skins>& info) {
                return skin == info.second && info.first;
            }))
                return skinId;
        }
        return 0;
    }

    void SkinModifier::OnEntityAppear(app::Proto_SceneEntityAppearNotify* notify)
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
                    avatar->fields.costumeId_ = GetSkinId(avatar->fields.avatarId_);
            }
        }
    }
}