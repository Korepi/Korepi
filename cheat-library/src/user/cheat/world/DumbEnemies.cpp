#include "pch-il2cpp.h"
#include "DumbEnemies.h"

#include <helpers.h>

namespace cheat::feature 
{
    static void VCMonsterAIController_TryDoSkill_Hook(void* __this, uint32_t skillID, MethodInfo* method);

    DumbEnemies::DumbEnemies() : Feature(),
        NFP(f_Enabled, "DumbEnemies", "Dumb Enemies", false)
    {
		HookManager::install(app::MoleMole_VCMonsterAIController_TryDoSkill, VCMonsterAIController_TryDoSkill_Hook);
    }

    const FeatureGUIInfo& DumbEnemies::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("World");
        return info;
    }

    void DumbEnemies::DrawMain()
    {
        ConfigWidget(_TR("Dumb Enemies"), f_Enabled, _TR("Enemies don't attack or use abilities against player. \n"
            "May not work with some enemies or enemy abilities."));
    }

    bool DumbEnemies::NeedStatusDraw() const
{
        return f_Enabled->enabled();
    }

    void DumbEnemies::DrawStatus() 
    { 
        ImGui::Text(_TR("Dumb Enemies"));
    }

    DumbEnemies& DumbEnemies::GetInstance()
    {
        static DumbEnemies instance;
        return instance;
    }

	// Raised when monster trying to do skill. Attack also is skill.
    // We just block if dumb mob enabled, so mob will not attack player.
	static void VCMonsterAIController_TryDoSkill_Hook(void* __this, uint32_t skillID, MethodInfo* method)
	{
        DumbEnemies& dumbEnemies = DumbEnemies::GetInstance();
		if (dumbEnemies.f_Enabled->enabled())
			return;
		CALL_ORIGIN(VCMonsterAIController_TryDoSkill_Hook, __this, skillID, method);
	}
}

