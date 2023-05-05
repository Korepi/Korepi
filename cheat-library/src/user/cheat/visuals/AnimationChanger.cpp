#include "pch-il2cpp.h"
#include "AnimationChanger.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
    static void MoleMole_PlayerModule_EntityAppear_Hook(app::MoleMole_PlayerModule* __this, app::Proto_SceneEntityInfo* entity, app::VisionType__Enum type, uint32_t infoParam, MethodInfo* method);
    std::vector<std::string> animations;

    AnimationChanger::AnimationChanger() : Feature(),
        NFP(f_Enabled, "Visuals::AnimationChanger", "Animation Changer", false),
        NF(f_Animation, "Visuals::AnimationChanger", "Attack01"),
        NF(f_ApplyKey, "Visuals::AnimationChanger", Hotkey('Y')),
        NF(f_ResetKey, "Visuals::AnimationChanger", Hotkey('R')),
        NF(f_Delay, "Visuals::AnimationChanger", 400),
        NF(f_Debug, "Visuals::AnimationChanger", false)
    {
        HookManager::install(app::MoleMole_PlayerModule_EntityAppear, MoleMole_PlayerModule_EntityAppear_Hook);
        events::GameUpdateEvent += MY_METHOD_HANDLER(AnimationChanger::OnGameUpdate);
    }

    const FeatureGUIInfo& AnimationChanger::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Animation Changer", "Visuals");
        return info;
    }

    void AnimationChanger::DrawMain()
    {
		ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Changes active character's animation."));
		if (f_Enabled->enabled())
		{
			if (ImGui::BeginCombo(_TR("Animations"), f_Animation.value().c_str()))
			{
				for (auto& animation : animations)
				{
					bool is_selected = (f_Animation.value().c_str() == animation);
					if (ImGui::Selectable(animation.c_str(), is_selected))
						f_Animation.value() = animation;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ConfigWidget(_TR("Apply Key"), f_ApplyKey, true);
			ConfigWidget(_TR("Reset Key"), f_ResetKey, true);
			ConfigWidget(_TR("Delay"), f_Delay, 1, 1, 1000000000, _TR("Delay to repeat animation"));
			ConfigWidget(_TR("Debug Animations"), f_Debug, _TR("Logs current active character's animation state."));
		}
    }

    bool AnimationChanger::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void AnimationChanger::DrawStatus()
    {
        ImGui::Text(_TR("Animation Changer"));
    }

    AnimationChanger& AnimationChanger::GetInstance()
    {
        static AnimationChanger instance;
        return instance;
    }

    // Taiga#5555: Called when you switch characters. Feel free to get rid of this hook if you find a better function.
    static void MoleMole_PlayerModule_EntityAppear_Hook(app::MoleMole_PlayerModule* __this, app::Proto_SceneEntityInfo* entity, app::VisionType__Enum type, uint32_t infoParam, MethodInfo* method)
    {
        auto& animationChanger = AnimationChanger::GetInstance();
        CALL_ORIGIN(MoleMole_PlayerModule_EntityAppear_Hook, __this, entity, type, infoParam, method);
        animationChanger.onEntityAppear = false;
    }

    // Feel free to refactor.
    void AnimationChanger::OnGameUpdate()
    {
        if (!f_Enabled->enabled())
            return;

        UPDATE_DELAY(f_Delay);

        auto& manager = game::EntityManager::instance();
        auto avatar = manager.avatar();
        if (avatar->animator() == nullptr)
            return;

        auto avatarName = avatar->name();
        //std::string objectName = il2cppi_to_string(app::Object_1_get_name(reinterpret_cast<app::Object_1*>(avatar->gameObject()), nullptr)).c_str();
        //LOG_DEBUG("avatarName = %s", avatarName.c_str());
        //LOG_DEBUG("objectName = %s", objectName.c_str());
        // path sample: EntityRoot/AvatarRoot/Avatar_Girl_Sword_Nilou(Clone)/OffsetDummy/Avatar_Girl_Sword_Nilou
        auto gameObj = app::GameObject_Find(string_to_il2cppi("EntityRoot/AvatarRoot/" + avatarName.substr(0, avatarName.find_first_of(" ")) + "/OffsetDummy/" + avatarName.substr(0, avatarName.find_first_of("("))), nullptr);
        if (gameObj == nullptr)
            return;

        auto acComponent = app::GameObject_GetComponentByName(gameObj, string_to_il2cppi("AnimatorController"), nullptr);
        if (acComponent == nullptr)
            return;

        auto stateNamesArray = reinterpret_cast<app::AnimatorController*>(acComponent)->fields._stateNames;

        if (f_Debug)
            LOG_DEBUG(il2cppi_to_string(app::Extension_GetCurrentStateName(avatar->animator(), 0, reinterpret_cast<app::AnimatorController*>(acComponent), nullptr)).c_str());

        static bool isFull = false;
        for (uint32_t i = 0; i < stateNamesArray->max_length && !isFull; i++)
        {
            if (animations.size() == stateNamesArray->max_length)
            {
                std::sort(animations.begin(), animations.end());
                animations.erase(unique(animations.begin(), animations.end()), animations.end());
                isFull = true;
                continue;
            }

            animations.push_back(il2cppi_to_string(stateNamesArray->vector[i]).c_str());
        }

        if (!onEntityAppear && isFull)
        {
            animations.clear();
            isFull = false;
            onEntityAppear = true;
        }

        if (f_ApplyKey.value().IsPressed())
            app::Animator_Play(avatar->animator(), string_to_il2cppi(f_Animation.value().c_str()), 0, 0, nullptr);

        if (f_ResetKey.value().IsPressed())
            app::Animator_Rebind(avatar->animator(), nullptr);
    }
}
