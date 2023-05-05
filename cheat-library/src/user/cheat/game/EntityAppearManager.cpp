#include "pch-il2cpp.h"
#include "EntityAppearManager.h"

#include <helpers.h>

#include <cheat/game/EntityManager.h>
#include <cheat/events.h>

namespace cheat::feature
{
    static void MoleMole_GadgetModule_DoOnGadgetStateNotify_Hook(app::GadgetModule* __this, uint32_t gadgetEntityId, uint32_t gadgetState, bool isEnableInteract, MethodInfo* method);
    static void MoleMole_LevelModule_OnSceneEntityAppear_Hook(app::LevelModule* __this, app::Proto_SceneEntityAppearNotify* notify, uint32_t JOODHPBLPMA, MethodInfo* method);
    static void MoleMole_LevelModule_OnSceneEntityAppearAsync_Hook(app::LevelModule* __this, app::Proto_SceneEntityAppearNotify* notify, uint32_t JOODHPBLPMA, MethodInfo* method);
    static std::mutex m_GadgetLock;

    EntityAppearManager::EntityAppearManager()
    {
        HookManager::install(app::MoleMole_GadgetModule_DoOnGadgetStateNotify, MoleMole_GadgetModule_DoOnGadgetStateNotify_Hook);
        HookManager::install(app::MoleMole_LevelModule_OnSceneEntityAppear, MoleMole_LevelModule_OnSceneEntityAppear_Hook);
        HookManager::install(app::MoleMole_LevelModule_OnSceneEntityAppearAsync, MoleMole_LevelModule_OnSceneEntityAppearAsync_Hook);

        events::EntityAppearEvent += MY_METHOD_HANDLER(EntityAppearManager::OnEntityAppear);
    }

    cheat::feature::EntityAppearManager& EntityAppearManager::GetInstance()
    {
        static EntityAppearManager Instance;
        return Instance;
    }

    void EntityAppearManager::OnEntityAppear(app::Proto_SceneEntityAppearNotify* notify)
    {
        auto entityList = notify->fields.entityList_;
        if (entityList == nullptr)
            return;
        for (int i = 0; i < entityList->fields.values->fields._size; i++)
        {
            auto& itemFields = entityList->fields.values->fields._items->vector[i]->fields;
            if (itemFields.entityCase_ == app::Proto_SceneEntityInfo_Proto_SceneEntityInfo_EntityOneofCase__Enum::Gadget)
            {
                auto gadget = CastTo<app::Proto_SceneGadgetInfo>(itemFields.entity_, *app::Proto_SceneGadgetInfo__TypeInfo);
                if (gadget != nullptr)
                {
                    auto& entityAppearManager = EntityAppearManager::GetInstance();
                    std::lock_guard<std::mutex> _lock(m_GadgetLock);
                    entityAppearManager.m_Gadgets[itemFields.entityId_] = {
                        itemFields.entityId_,
                        gadget->fields.gadgetId_,
                        gadget->fields.ownerEntityId_,
                        static_cast<app::Proto_GadgetBornType__Enum>(gadget->fields.bornType_),
                        static_cast<app::GadgetState__Enum>(gadget->fields.gadgetState_),
                        static_cast<app::GadgetType_Enum>(gadget->fields.gadgetType_),
                        gadget->fields.isEnableInteract_,
                        static_cast<app::Proto_SceneGadgetInfo_ContentOneofCase__Enum>(gadget->fields.contentCase_),
                        gadget };
                }
            }
        }
    }

    static void ProcessStateNotify(uint32_t gadgetEntityId, uint32_t gadgetState, bool isEnableInteract)
    {
        auto& entityAppearManager = EntityAppearManager::GetInstance();
        std::lock_guard<std::mutex> _lcok(m_GadgetLock);
        entityAppearManager.m_Gadgets[gadgetEntityId].gadgetState = static_cast<app::GadgetState__Enum>(gadgetState);
        entityAppearManager.m_Gadgets[gadgetEntityId].isEnableInteract = isEnableInteract;
    }

    void MoleMole_GadgetModule_DoOnGadgetStateNotify_Hook(app::GadgetModule* __this, uint32_t gadgetEntityId, uint32_t gadgetState, bool isEnableInteract, MethodInfo* method)
    {
        ProcessStateNotify(gadgetEntityId, gadgetState, isEnableInteract);
        CALL_ORIGIN(MoleMole_GadgetModule_DoOnGadgetStateNotify_Hook, __this, gadgetEntityId, gadgetState, isEnableInteract, method);
    }

    void MoleMole_LevelModule_OnSceneEntityAppear_Hook(app::LevelModule* __this, app::Proto_SceneEntityAppearNotify* notify, uint32_t JOODHPBLPMA, MethodInfo* method)
    {
        SAFE_BEGIN();
        events::EntityAppearEvent(notify);
        SAFE_EEND();
        CALL_ORIGIN(MoleMole_LevelModule_OnSceneEntityAppear_Hook, __this, notify, JOODHPBLPMA, method);
    }

    void MoleMole_LevelModule_OnSceneEntityAppearAsync_Hook(app::LevelModule* __this, app::Proto_SceneEntityAppearNotify* notify, uint32_t JOODHPBLPMA, MethodInfo* method)
    {
        SAFE_BEGIN();
        events::EntityAppearEvent(notify);
        SAFE_EEND();
        CALL_ORIGIN(MoleMole_LevelModule_OnSceneEntityAppearAsync_Hook, __this, notify, JOODHPBLPMA, method);
    }
}