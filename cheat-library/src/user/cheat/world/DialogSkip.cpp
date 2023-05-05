#include "pch-il2cpp.h"
#include "DialogSkip.h"

#include <helpers.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
    static void InLevelCutScenePageContext_UpdateView_Hook(app::InLevelCutScenePageContext* __this, MethodInfo* method);
    static void InLevelCutScenePageContext_ClearView_Hook(app::InLevelCutScenePageContext* __this, MethodInfo* method);
    static void CriwareMediaPlayer_Update(app::CriwareMediaPlayer* __this, MethodInfo* method);
    static void MoleMole_CurtainClickContinueTask_Start_Hook(void* __this, MethodInfo* method);

    DialogSkip::DialogSkip() : Feature(),
        NFP(f_Enabled, "AutoTalk", "Auto Talk", false),
        NF(f_ToggleHotkey, "AutoTalk", Hotkey()),
        NFP(f_AutoSelectDialog, "AutoTalk", "Auto-select Dialog", true),
        NFP(f_ExcludeImportant, "AutoTalk", "Exclude Important", true),
        NFP(f_FastDialog, "AutoTalk", "Fast Dialog", false),
        NFP(f_CutsceneUSM, "AutoTalk", "Skip Cutscenes", false),
        NF(f_TimeSpeedup, "AutoTalk", 5.0f)
    {
        HookManager::install(app::MoleMole_InLevelCutScenePageContext_UpdateView, InLevelCutScenePageContext_UpdateView_Hook);
        HookManager::install(app::MoleMole_InLevelCutScenePageContext_ClearView, InLevelCutScenePageContext_ClearView_Hook);
        HookManager::install(app::CriwareMediaPlayer_Update, CriwareMediaPlayer_Update);
        HookManager::install(app::MoleMole_CurtainClickContinueTask_Start, MoleMole_CurtainClickContinueTask_Start_Hook);
    }

    const FeatureGUIInfo& DialogSkip::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Auto Talk", "World");
        return info;
    }

    void DialogSkip::DrawMain()
    {
        ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Automatically continue the dialog."));
        if (f_Enabled->enabled())
        { 
            ConfigWidget(_TR("Toggle Hotkey"), f_ToggleHotkey, true, _TR("Change behavior to a held down toggle if bound to a key.\nLeave as 'None' for default behavior (always on)."));
        }

        ConfigWidget(_TR("Auto-select Dialog"), f_AutoSelectDialog, _TR("Automatically select dialog choices."));
        if (f_AutoSelectDialog->enabled())
        {
            ImGui::Indent();
            ConfigWidget(_TR("Exclude Katheryne/Tubby/Wagner"), f_ExcludeImportant, _TR("Exclude Kath/Tubby/Wagner from auto-select."));
            ImGui::Unindent();
        }

        ConfigWidget(_TR("Fast Dialog"), f_FastDialog, _TR("Speeds up Time"));
        if (f_FastDialog->enabled())
        {
            ConfigWidget(_TR("Time Speed"), f_TimeSpeedup, 0.1f, 2.0f, 50.0f, _TR("Time Speedup Multipler \nHigher Values will lead to sync issues with servers \nand is not recommended for Laggy Internet connections."));
        }

        ConfigWidget(_TR("Skip Cutscenes"), f_CutsceneUSM, _TR("Automatically skips game movies."));
    }

    bool DialogSkip::NeedStatusDraw() const
    {
        return f_Enabled->enabled() || f_CutsceneUSM->enabled();
    }

    void DialogSkip::DrawStatus()
    {
        if (f_Enabled->enabled())
            ImGui::Text("%s [%s%s%s%s%s]",
                _TR("Dialog"),
                f_AutoSelectDialog->enabled() ? _TR("Auto") : _TR("Manual"),
                f_AutoSelectDialog->enabled() && (f_ExcludeImportant->enabled() || f_FastDialog->enabled()) ? "|" : "",
                f_ExcludeImportant->enabled() ? _TR("Exc") : "",
                f_ExcludeImportant->enabled() && f_FastDialog->enabled() ? "|" : "",
                f_FastDialog->enabled() ? _TR("Fast") : _TR("Normal"));

        if (f_CutsceneUSM->enabled() && f_Enabled->enabled())
            ImGui::Text(_TR("Skip Cutscenes"));
    }

    DialogSkip& DialogSkip::GetInstance()
    {
        static DialogSkip instance;
        return instance;
    }

    static void ResetGamespeed()
    {
        float gameSpeed = app::Time_get_timeScale(nullptr);
        if (gameSpeed > 1.0f)
            app::Time_set_timeScale(1.0f, nullptr);
    }

    // Raised when dialog view updating
    // We call free click, if auto talk enabled, that means we just emulate user click
    // When appear dialog choose we create notify with dialog select first item.
    void DialogSkip::OnCutScenePageUpdate(app::InLevelCutScenePageContext* context)
    {
        if (!f_Enabled->enabled() || !f_ToggleHotkey->IsPressed())
        {
            ResetGamespeed();
            return;
        }

        auto talkDialog = context->fields._talkDialog;
        if (talkDialog == nullptr)
            return;

        if (f_FastDialog->enabled())
            app::Time_set_timeScale(f_TimeSpeedup, nullptr);
        else
            app::Time_set_timeScale(1.0f, nullptr);

        bool isImportant = false;
        if (f_ExcludeImportant->enabled())
        {
            // TODO: Add a custom filter in the future where users can
            // add their own name substrings of entities to avoid
            // speeding up dialog on.
            std::vector<std::string> impEntitiesNames = {
                "Djinn",
                "Katheryne",
                "Wagner",
                "Ahangar",
                "MasterZhang",
                "Cat_Prince"
            };
            auto dialogPartnerID = context->fields._inteeID;
            auto& manager = game::EntityManager::instance();
            auto dialogPartner = manager.entity(dialogPartnerID);
            auto dialogPartnerName = dialogPartner->name();
            for (auto impEntityName : impEntitiesNames)
            {
                if (dialogPartnerName.find(impEntityName) != -1) {
                    //LOG_DEBUG("%s %s %d", dialogPartnerName.c_str(), impEntityName, dialogPartnerName.find(impEntityName));
                    isImportant = true;
                    break;
                }
            }
        }

        if (talkDialog->fields._inSelect && f_AutoSelectDialog->enabled() && !isImportant)
        {
            int32_t value = 0;
            auto object = il2cpp_value_box((Il2CppClass*)*app::Int32__TypeInfo, &value);
            app::Notify notify{};
            notify.type = app::MoleMole_NotifyTypes__Enum::DialogSelectNotify;
            notify.body = (app::Object*)object;
            app::MoleMole_TalkDialogContext_OnDialogSelectItem(talkDialog, &notify, nullptr);
        }
        else if (!talkDialog->fields._inSelect)
            app::MoleMole_InLevelCutScenePageContext_OnFreeClick(context, nullptr);
    }

    static void InLevelCutScenePageContext_UpdateView_Hook(app::InLevelCutScenePageContext* __this, MethodInfo* method)
    {
        CALL_ORIGIN(InLevelCutScenePageContext_UpdateView_Hook, __this, method);

        DialogSkip& dialogSkip = DialogSkip::GetInstance();
        dialogSkip.OnCutScenePageUpdate(__this);
    }

    // Raised when exiting a dialog. We try to hackishly return to normal value.
    // Should be a better way to store the pre-dialog speed using Time_get_timeScale.
    static void InLevelCutScenePageContext_ClearView_Hook(app::InLevelCutScenePageContext* __this, MethodInfo* method)
    {
        ResetGamespeed();
        CALL_ORIGIN(InLevelCutScenePageContext_ClearView_Hook, __this, method);
    }

    static void CriwareMediaPlayer_Update(app::CriwareMediaPlayer* __this, MethodInfo* method)
    {
        DialogSkip& dialogSkip = DialogSkip::GetInstance();
        if (dialogSkip.f_CutsceneUSM->enabled())
            app::CriwareMediaPlayer_Skip(__this, nullptr);

        return CALL_ORIGIN(CriwareMediaPlayer_Update, __this, method);
    }

    //CNLouisLiu
    static void MoleMole_CurtainClickContinueTask_Start_Hook(void* __this, MethodInfo* method)
    {
        DialogSkip& dialogSkip = DialogSkip::GetInstance();
        if (!dialogSkip.f_Enabled->enabled()) {
            CALL_ORIGIN(MoleMole_CurtainClickContinueTask_Start_Hook, __this, method);
            return;
        }
        app::MoleMole_CurtainClickContinueTask_Finish(__this, NULL);
    }
}

