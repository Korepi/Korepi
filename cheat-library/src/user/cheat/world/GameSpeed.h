#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{

    class GameSpeed : public Feature
    {
    public:
        config::Field<TranslatedHotkey> f_Enabled;
        config::Field<Hotkey> f_Hotkey;
        config::Field<float> f_Speed;

        static GameSpeed& GetInstance();

        const FeatureGUIInfo& GetGUIInfo() const override;
        void DrawMain() override;

        virtual bool NeedStatusDraw() const override;
        void DrawStatus() override;

        void OnGameUpdate();
    private:
        GameSpeed();
    };
}