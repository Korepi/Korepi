#include "pch-il2cpp.h"
#include "MobVacuum.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>

namespace cheat::feature 
{
    MobVacuum::MobVacuum() : Feature(),
        NFP(f_Enabled, "MobVacuum", "Mob Vacuum", false),
        NFP(f_IncludeMonsters, "MobVacuum", "Include monsters", true),
        NF(f_MonsterCommon, "MobVacuum", true),
        NF(f_MonsterElites, "MobVacuum", true),
        NF(f_MonsterBosses, "MobVacuum", true),
        NFP(f_IncludeAnimals, "MobVacuum", "Include animals", true),
        NF(f_AnimalDrop, "MobVacuum", true),
        NF(f_AnimalPickUp, "MobVacuum", true),
        NF(f_AnimalNPC, "MobVacuum", true),
        NF(f_Speed, "MobVacuum", 2.5f),
        NF(f_Distance, "MobVacuum", 1.5f),
        NF(f_Radius, "MobVacuum", 10.0f),
        NF(f_OnlyTarget, "MobVacuum", true),
        NF(f_Instantly, "MobVacuum", false),
        NFP(f_SetCollider, "MobVacuum", "Set collider", false)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(MobVacuum::OnGameUpdate);
        events::MoveSyncEvent += MY_METHOD_HANDLER(MobVacuum::OnMoveSync);
    }

    const FeatureGUIInfo& MobVacuum::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Mob Vacuum", "World");
        return info;
    }

    void MobVacuum::DrawMain()
    {
        ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Enables mob vacuum.\n" \
            "Mobs within the specified radius will move\nto a specified distance in front of the player."));

        bool filtersChanged = false;
        ImGui::BeginGroupPanel(_TR("Monsters"));
        {
            filtersChanged |= ConfigWidget(_TR("Include Monsters"), f_IncludeMonsters, _TR("Include monsters in vacuum."));
            filtersChanged |= ConfigWidget(_TR("Common"), f_MonsterCommon, _TR("Common enemies.")); ImGui::SameLine();
            filtersChanged |= ConfigWidget(_TR("Elite"), f_MonsterElites, _TR("Elite enemies.")); ImGui::SameLine();
            filtersChanged |= ConfigWidget(_TR("Boss"), f_MonsterBosses, _TR("World and Trounce boss enemies."));
        }
        ImGui::EndGroupPanel();
        
        ImGui::BeginGroupPanel(_TR("Animals"));
        {
            filtersChanged |= ConfigWidget(_TR("Include Animals"), f_IncludeAnimals, _TR("Include animals in vacuum."));
            filtersChanged |= ConfigWidget(_TR("Droppers"), f_AnimalDrop, _TR("Animals you need to kill before collecting.")); ImGui::SameLine();
            filtersChanged |= ConfigWidget(_TR("Pick-ups"), f_AnimalPickUp, _TR("Animals you can immediately collect.")); ImGui::SameLine();
            filtersChanged |= ConfigWidget(_TR("NPCs"), f_AnimalNPC, _TR("Animals without mechanics."));
        }
        ImGui::EndGroupPanel();

        if (filtersChanged)
            UpdateFilters();

    	ConfigWidget(_TR("Instant Vacuum"), f_Instantly, _TR("Vacuum entities instantly."));
        ConfigWidget(_TR("Only Hostile/Aggro"), f_OnlyTarget, _TR("If enabled, vacuum will only affect monsters targeting you. Will not affect animals."));
        ConfigWidget(_TR("Remove Collider"), f_SetCollider, _TR("If enabled, monsters won't be able to push you despite the distance or size"));
        ConfigWidget(_TR("Speed"), f_Speed, 0.1f, 1.0f, 15.0f, _TR("If 'Instant Vacuum' is not checked, mob will be vacuumed at the specified speed."));
        ConfigWidget(_TR("Radius (m)"), f_Radius, 0.1f, 5.0f, 150.0f, _TR("Radius of vacuum."));
        ConfigWidget(_TR("Distance (m)"), f_Distance, 0.1f, 0.5f, 10.0f, _TR("Distance between the player and the monster."));
    }

    bool MobVacuum::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void MobVacuum::DrawStatus() 
    { 
        ImGui::Text("%s [%s]\n[%s|%.01fm|%.01fm|%s|%s]",
            _TR("Mob Vacuum"),
            f_IncludeMonsters->enabled() && f_IncludeAnimals->enabled() ? _TR("All") : f_IncludeMonsters ? _TR("Monsters") : f_IncludeAnimals ? _TR("Animals") : _TR("None"),
            f_Instantly ? _TR("Instant") : fmt::format("{}|{:.1f}", _TR("Normal"), f_Speed.value()).c_str(),
            f_Radius.value(),
            f_Distance.value(),
            f_OnlyTarget ? _TR("Aggro") : _TR("All"),
            f_SetCollider->enabled() ? _TR("RC") : "");
    }

    MobVacuum& MobVacuum::GetInstance()
    {
        static MobVacuum instance;
        return instance;
    }

    // Combines selected vacuum filters.
    void MobVacuum::UpdateFilters() {
        
        m_Filters.clear();

        if (f_IncludeMonsters->enabled()) 
        {
            if (f_MonsterCommon) m_Filters.push_back(&game::filters::combined::MonsterCommon);
            if (f_MonsterElites) m_Filters.push_back(&game::filters::combined::MonsterElites);
            if (f_MonsterBosses) m_Filters.push_back(&game::filters::combined::MonsterBosses);
        }

        if (f_IncludeAnimals->enabled()) 
        {
            if (f_AnimalDrop) m_Filters.push_back(&game::filters::combined::AnimalDrop);
            if (f_AnimalPickUp) m_Filters.push_back(&game::filters::combined::AnimalPickUp);
            if (f_AnimalNPC) m_Filters.push_back(&game::filters::combined::AnimalNPC);
        }
    }

    // Check if entity valid for mob vacuum.
    bool MobVacuum::IsEntityForVac(game::Entity* entity)
    {
        if (m_Filters.empty())
            return false;

        bool entityValid = std::any_of(m_Filters.cbegin(), m_Filters.cend(), 
            [entity](const game::IEntityFilter* filter) { return filter->IsValid(entity); });
        if (!entityValid)
            return false;

        auto& manager = game::EntityManager::instance();
        if (f_OnlyTarget && game::filters::combined::Monsters.IsValid(entity))
        {
            auto monsterCombat = entity->combat();
            if (monsterCombat == nullptr || monsterCombat->fields._attackTarget.runtimeID != manager.avatar()->runtimeID())
                return false;
        }

		auto distance = manager.avatar()->distance(entity);
        return distance <= f_Radius;
    }

    // Calculate mob vacuum target position.
    app::Vector3 MobVacuum::CalcMobVacTargetPos()
    {
        auto& manager = game::EntityManager::instance();
        auto avatarEntity = manager.avatar();
        if (avatarEntity == nullptr)
            return {};

        return avatarEntity->relativePosition() + avatarEntity->forward() * f_Distance;
    }

    // Set Monster's collider
    // Taiga#5555: There might be an in-game function for this already I'm just not sure which one
    void SetMonsterCollider(bool v)
    {
        UPDATE_DELAY(300);

        auto monsterRoot = app::GameObject_Find(string_to_il2cppi("/EntityRoot/MonsterRoot"), nullptr);
        if (monsterRoot != nullptr)
        {
            auto transform = app::GameObject_GetComponentByName(monsterRoot, string_to_il2cppi("Transform"), nullptr);
            auto monsterCount = app::Transform_get_childCount(reinterpret_cast<app::Transform*>(transform), nullptr);
            for (int i = 0; i <= monsterCount - 1; i++)
            {
                auto monsters = app::Transform_GetChild(reinterpret_cast<app::Transform*>(transform), i, nullptr);
                auto monsterGameObject = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(monsters), nullptr);
                auto monsterTransform = app::GameObject_GetComponentByName(monsterGameObject, string_to_il2cppi("Transform"), nullptr);
                auto transformChild = app::Transform_FindChild(reinterpret_cast<app::Transform*>(monsterTransform), string_to_il2cppi("Collider"), nullptr);
                auto colliderGameObject = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(transformChild), nullptr);
                if (app::GameObject_get_active(colliderGameObject, nullptr) == v)
                    continue;
                app::GameObject_SetActive(colliderGameObject, v, nullptr);
            }
        }
    }

    // Mob vacuum update function.
    // Changes position of monster, if mob vacuum enabled.
    void MobVacuum::OnGameUpdate()
    {
        if (!f_Enabled->enabled())
            return;

        app::Vector3 targetPos = CalcMobVacTargetPos();
        if (IsVectorZero(targetPos))
            return;

        UpdateFilters();
        if (!f_IncludeMonsters->enabled() && !f_IncludeAnimals->enabled())
            return;

        if (m_Filters.empty())
            return;

        auto& manager = game::EntityManager::instance();
        for (const auto& entity : manager.entities())
        {
            if (!IsEntityForVac(entity))
                continue;

            SetMonsterCollider(!f_SetCollider->enabled());

            if (f_Instantly)
            {
                entity->setRelativePosition(targetPos);
                continue;
            }

            app::Vector3 entityRelPos = entity->relativePosition();
            if (app::Vector3_Distance(entityRelPos, targetPos, nullptr) > 0.1)
            {
                app::Vector3 dir = GetVectorDirection(entityRelPos, targetPos);
                float deltaTime = app::Time_get_deltaTime(nullptr);
                app::Vector3 newPosition = entityRelPos + dir * f_Speed * deltaTime;
                entity->setRelativePosition(newPosition);
            }
        }
    }

    // Mob vacuum sync packet replace.
    // Replacing move sync speed and motion state.
    //   Callow: I think it is more safe method, 
    //           because for server monster don't change position instantly.
    void MobVacuum::OnMoveSync(uint32_t entityId, app::MotionInfo* syncInfo)
    {
        if (!f_Enabled->enabled() || f_Instantly)
            return;

        auto& manager = game::EntityManager::instance();
        auto entity = manager.entity(entityId);
        if (!IsEntityForVac(entity))
            return;

        SetMonsterCollider(!f_SetCollider->enabled());

        app::Vector3 targetPos = CalcMobVacTargetPos();
        app::Vector3 entityPos = entity->relativePosition();
        if (app::Vector3_Distance(targetPos, entityPos, nullptr) < 0.2)
            return;

        app::Vector3 dir = GetVectorDirection(targetPos, entityPos);
        app::Vector3 scaledDir = dir * f_Speed;

        syncInfo->fields.speed_->fields.x = scaledDir.x;
        syncInfo->fields.speed_->fields.y = scaledDir.y;
        syncInfo->fields.speed_->fields.z = scaledDir.z;

        switch (syncInfo->fields.motionState)
        {
        case app::MotionState__Enum::MotionStandby:
        case app::MotionState__Enum::MotionStandbyMove:
        case app::MotionState__Enum::MotionWalk:
        case app::MotionState__Enum::MotionDangerDash:
            syncInfo->fields.motionState = app::MotionState__Enum::MotionRun;
        }
    }
}

