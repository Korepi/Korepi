#include "pch-il2cpp.h"
#include "EnablePeeking.h"

#include <helpers.h>

namespace cheat::feature
{
    static void MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook(app::MoleMole_VCBaseSetDitherValue* __this, float value, MethodInfo* method);

    EnablePeeking::EnablePeeking() : Feature(),
        NFP(f_Enabled, "Visuals::EnablePeeking", "Enable Peeking", false)
    {
        HookManager::install(app::MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue, MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook);
    }

    const FeatureGUIInfo& EnablePeeking::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("Visuals");
        return info;
    }

    void EnablePeeking::DrawMain()
    {
        ConfigWidget(_TR("Enable Peeking"), f_Enabled, ";)");
    }

    bool EnablePeeking::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void EnablePeeking::DrawStatus()
    {
        ImGui::Text(_TR("Enable Peeking"));
    }

    EnablePeeking& EnablePeeking::GetInstance()
    {
        static EnablePeeking instance;
        return instance;
    }

    static void MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook(app::MoleMole_VCBaseSetDitherValue* __this, float value, MethodInfo* method)
    {
        EnablePeeking& EnablePeeking = EnablePeeking::GetInstance();
        if (EnablePeeking.f_Enabled->enabled())
            value = 1;

        CALL_ORIGIN(MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook, __this, value, method);
    }
}

