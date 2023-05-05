#include "pch-il2cpp.h"
#include "GodMode.h"

#include <helpers.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature 
{
    GodMode::GodMode() : Feature(),
        NFP(f_Enabled, "GodMode", "God Mode", false),
        NF(f_Conditional, "GodMode", false),
        NF(f_MinHealth, "GodMode", 95.0f),
        NF(f_MissingRate, "GodMode", 100.0f)
    {
		HookManager::install(app::VCHumanoidMove_NotifyLandVelocity, VCHumanoidMove_NotifyLandVelocity_Hook);
		HookManager::install(app::Miscs_CheckTargetAttackable, Miscs_CheckTargetAttackable_Hook);
        //HookManager::install(app::MoleMole_LCBaseCombat_FireBeingHitEvent, LCBaseCombat_FireBeingHitEvent_Hook);
        HookManager::install(app::MoleMole_ActorAbilityPlugin_HanlderModifierThinkTimerUp, MoleMole_ActorAbilityPlugin_HanlderModifierThinkTimerUp_Hook);
    }

    const FeatureGUIInfo& GodMode::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("God Mode", "Player");
        return info;
    }

    void GodMode::DrawMain()
    {
        ConfigWidget(_TR("God Mode"), f_Enabled, _TR("Enables god mode, i.e. no incoming damage including environmental damage.\n"));
        ImGui::Indent();
        ConfigWidget(_TR("Conditional"), f_Conditional, _TR("Enables god mode when the character's health drops\n"
            "below the specified minimum.\n"));
        if (f_Conditional) 
        {
            ConfigWidget(_TR("Minimum Health"), f_MinHealth, 0.1f, 0.1f, 100.0f, _TR("Minimum health (in %) required before god mode takes effect."));
            ConfigWidget(_TR("Missing Attack Rate"), f_MissingRate, 0.1f, 0.0f, 100.0f, _TR("Randomly missing enemies attack (in %, 0% = never, 100% = always) such as evade ability in other games."));
        }
            
        ImGui::Unindent();
    }

    bool GodMode::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void GodMode::DrawStatus() 
    {
        ImGui::Text(_TR("God Mode"));
        if (f_Conditional)
            ImGui::Text("%s MH%0.2f%% | MR%0.2f%%", _TR("Condition"), f_MinHealth.value(), f_MissingRate.value());
    }

    GodMode& GodMode::GetInstance()
    {
        static GodMode instance;
        return instance;
    }

    bool HealthLowThanMin(game::Entity avatar)
    {
        auto& gm = GodMode::GetInstance();
        auto combat = avatar.combat();
        if (combat == nullptr)
            return false;

        float health = app::MoleMole_SafeFloat_get_Value(combat->fields._combatProperty_k__BackingField->fields.HP, nullptr);
        float maxHealth = app::MoleMole_SafeFloat_get_Value(combat->fields._combatProperty_k__BackingField->fields.maxHP, nullptr);
        bool isLowThanMin = 100.0f - (100.0f - ((health / maxHealth) * 100.0f)) <= gm.f_MinHealth.value();
        return isLowThanMin;
    }

    bool IsMissingAttack()
    {
        auto& gm = GodMode::GetInstance();
        if (!gm.f_Conditional)
            return true;
        if (gm.f_MissingRate >= 100.0f)
            return true;
        else if (gm.f_MissingRate <= 0.0f)
            return false;
        else 
        {
            float rand = (float)(std::rand() % 1000) / 10.0f;
            return rand <= gm.f_MissingRate.value();
        }
    }

	// Attack immunity (return false when target is avatar, that mean avatar entity isn't attackable)
	bool GodMode::Miscs_CheckTargetAttackable_Hook(app::BaseEntity* attacker, app::BaseEntity* target, MethodInfo* method)
	{
        auto& gm = GodMode::GetInstance();
        auto& manager = game::EntityManager::instance();
        auto entity = manager.entity(target);
        if (gm.f_Enabled->enabled() && entity->isAvatar())
            if (gm.f_Conditional) { // Calculate only when conditional is enabled
                if (IsMissingAttack() || HealthLowThanMin(target))
                    return false;
            }
            else
                return false;

		return CALL_ORIGIN(Miscs_CheckTargetAttackable_Hook, attacker, target, method);
	}

	// Raised when avatar fall on ground.
    // Sending fall speed, and how many time pass from gain max fall speed (~30m/s).
    // To disable fall damage reset reachMaxDownVelocityTime and decrease fall velocity.
	void GodMode::VCHumanoidMove_NotifyLandVelocity_Hook(app::VCHumanoidMove* __this, app::Vector3 velocity, float reachMaxDownVelocityTime, MethodInfo* method)
	{
        auto& gm = GodMode::GetInstance();
		if (gm.f_Enabled->enabled() && -velocity.y > 13)
		{
            if (gm.f_Conditional)
            {
                if (HealthLowThanMin(__this->fields._._._._entity))
                {
                    float randAdd = (float)(std::rand() % 1000) / 1000;
                    velocity.y = -8 - randAdd;
                    reachMaxDownVelocityTime = 0;
                }
            }
            else {
                float randAdd = (float)(std::rand() % 1000) / 1000;
                velocity.y = -8 - randAdd;
                reachMaxDownVelocityTime = 0;
            }
		}
		// LOG_DEBUG("%s, %f", il2cppi_to_string(velocity).c_str(), reachMaxDownVelocityTime);
		CALL_ORIGIN(VCHumanoidMove_NotifyLandVelocity_Hook, __this, velocity, reachMaxDownVelocityTime, method);
	}

    // Analog function for disable attack damage (Thanks to Taiga74164)
    //void GodMode::LCBaseCombat_FireBeingHitEvent_Hook(app::LCBaseCombat* __this, uint32_t attackeeRuntimeID, app::AttackResult* attackResult, MethodInfo* method)
    //{        
    //    auto& gm = GodMode::GetInstance();
    //    auto& manager = game::EntityManager::instance();
    //    if (gm.f_AltGodMode && manager.avatar()->runtimeID() == attackeeRuntimeID)
    //        return;

    //    CALL_ORIGIN(LCBaseCombat_FireBeingHitEvent_Hook, __this, attackeeRuntimeID, attackResult, method);
    //}

    // Environmental damage immunity (Thanks to RELOADED#7236 / GitHub: @34736384)
    bool GodMode::MoleMole_ActorAbilityPlugin_HanlderModifierThinkTimerUp_Hook(app::ActorAbilityPlugin* __this, float delay, app::Object* arg, MethodInfo* method)
    {
        auto& gm = GodMode::GetInstance();

        if (GetInstance().NeedBlockHanlerModifierThinkTimeUp(arg))
            if (gm.f_Conditional) {
                if (HealthLowThanMin(__this->fields._.owner->fields._entity) && IsMissingAttack())
                    return false;
            }
            else 
                return false;

        return CALL_ORIGIN(MoleMole_ActorAbilityPlugin_HanlderModifierThinkTimerUp_Hook, __this, delay, arg, method);
    }

	bool GodMode::NeedBlockHanlerModifierThinkTimeUp(app::Object* arg)
	{
        if (!f_Enabled->enabled())
            return false;

		auto actorModifier = CastTo<app::MoleMole_ActorModifier>(arg, *app::MoleMole_ActorModifier__TypeInfo);
        if (actorModifier == nullptr)
            return false;

		static std::vector<std::string> modifierBlacklist
		{
			"BlackMud",
			"SERVER_ClimateAbility",
			"ElectricWater",
			"SeiraiThunder",
			"UNIQUE_Monster_",
			"Monster_Shougun",
            "UNIQUE_Life_Point_Search",
            "UNIQUE_DynamicAbility_DeathZone_LoseHp",
            "Monster_Nada_Stage02_Doom_Damage"
		};
        
        std::string modifierName = il2cppi_to_string(actorModifier->fields._config->fields._modifierName);
        //LOG_DEBUG("modifierName = %s", modifierName.c_str());
		for (auto& forbiddenModifierName : modifierBlacklist)
			if (modifierName.find(forbiddenModifierName) != std::string::npos)
				return true;

        return false;
	}
}