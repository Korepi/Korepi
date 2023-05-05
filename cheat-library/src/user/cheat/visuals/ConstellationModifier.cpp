#include "pch-il2cpp.h"
#include "ConstellationModifier.h"

#include <cheat/game/filters.h>
#include <cheat/events.h>

namespace cheat::feature
{

    ConstellationModifier::ConstellationModifier() : Feature(),
        NFP(f_Enabled, "ConstellationModifier", "Constellation Modifier", false)
    {
        InitConstellations();

        events::EntityAppearEvent += MY_METHOD_HANDLER(ConstellationModifier::OnEntityAppear);
    }

    ConstellationModifier& ConstellationModifier::GetInstance()
    {
        static ConstellationModifier Instance;
        return Instance;
    }

    const FeatureGUIInfo& ConstellationModifier::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Constellation Modifier", "Visuals");
        return info;
    }

    void ConstellationModifier::DrawMain()
    {
        int line = 1;
        ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Change map apply."));
        ImGui::PushID("Constellation");

        bool checked = std::all_of(m_Talents.begin(), m_Talents.end(), [](const std::pair<config::Field<bool>, Avatars>& talent) { return talent.first; });
        bool changed = false;
        if (ImGui::BeginSelectableGroupPanel(Translator::RuntimeTranslate("Constellation").c_str(), checked, changed, true))
        {
            for (auto& talent : m_Talents)
            {
                ImGui::PushID(&talent);
                ConfigWidget(Translator::RuntimeTranslate(magic_enum::enum_name(talent.second).data()).c_str(), talent.first);
                if (line % 5 != 0)
                {
                    ImGui::SameLine();
                }
                line++;
                ImGui::PopID();
            }
        }
        ImGui::EndSelectableGroupPanel();

        if (changed)
        {
            for (const auto& info : m_Talents)
            {
                info.first.value() = checked;
                info.first.FireChanged();
            }
        }

        ImGui::PopID();
    }

    bool ConstellationModifier::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void ConstellationModifier::DrawStatus()
    {
        ImGui::Text(_TR("Constellation Modifier"));
    }

    void ConstellationModifier::InitConstellations()
    {
        for (auto& [_, talents] : m_AllTalents)
        {
            m_Talents.push_back({ config::CreateField<bool>(magic_enum::enum_name(talents.first).data(), "ConstellationModifier", false, false), talents.first });
        }
    }

    app::UInt32__Array* ConstellationModifier::GetTalent(uint32_t avatarId)
    {
        auto newArray = (app::UInt32__Array*)il2cpp_array_new((Il2CppClass*)*app::UInt32__TypeInfo, 6);
        auto talent = m_AllTalents.find(avatarId);
        if (talent == m_AllTalents.end())
            return newArray;
        if (std::any_of(m_Talents.begin(), m_Talents.end(), [&](std::pair<config::Field<bool>, Avatars>& info) {
            return talent->second.first == info.second && info.first;
            }))
            memmove_s(newArray->vector, 24, talent->second.second.data(), 24);
            return newArray;
    }

    void ConstellationModifier::OnEntityAppear(app::Proto_SceneEntityAppearNotify* notify)
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
                if (avatar != nullptr && GetTalent(avatar->fields.avatarId_)->vector[0] != 0)
                {
                    avatar->fields.talentIdList_->fields.count = 6;
                    avatar->fields.talentIdList_->fields._array = GetTalent(avatar->fields.avatarId_);
                }
            }
        }
    }
}