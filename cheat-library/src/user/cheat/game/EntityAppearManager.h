#pragma once
#include <cheat-base/config/config.h>

namespace cheat::feature
{
    class EntityAppearManager
    {
    public:
        struct GadgetInfo
        {
            uint32_t entityId = 0;
            uint32_t gadgetId = 0;
            uint32_t ownerEntityId = 0;
            app::Proto_GadgetBornType__Enum bornType = app::Proto_GadgetBornType__Enum::GadgetBornNone;
            app::GadgetState__Enum gadgetState = app::GadgetState__Enum::Default;
            app::GadgetType_Enum gadgetType = app::GadgetType_Enum::GADGET_NONE;
            bool isEnableInteract = false;
            app::Proto_SceneGadgetInfo_ContentOneofCase__Enum contentCase = app::Proto_SceneGadgetInfo_ContentOneofCase__Enum::None;
            app::Proto_SceneGadgetInfo* data = nullptr;
        };

        std::unordered_map<uint32_t, GadgetInfo> m_Gadgets = {};

        static EntityAppearManager& GetInstance();
    private:
        void OnEntityAppear(app::Proto_SceneEntityAppearNotify* notify);
        EntityAppearManager();
    };
}