#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{
    class FlycloakModifier : public Feature
    {
    public:
        // constexpr when init with a value :(
        // fk invisible str
        enum class FlycloakType
        {
            FirstFlight,
            Companionship,
            Descension,
            AzureWind,
            GoldenFlight,
            ConcealingSnow,
            Feasting,
            TheStormstrider,
            ShimmeringGalaxy,
            TheForest,
            StarlitFeast,
        };

        std::map<FlycloakType, uint32_t> flycloakType
        {
            { FlycloakType::FirstFlight, 140001 },
            { FlycloakType::Companionship, 140002 },
            { FlycloakType::Descension, 140003 },
            { FlycloakType::AzureWind, 140004 },
            { FlycloakType::GoldenFlight, 140005 },
            { FlycloakType::ConcealingSnow, 140006 },
            { FlycloakType::Feasting, 140007 },
            { FlycloakType::TheStormstrider, 140008 },
            { FlycloakType::ShimmeringGalaxy, 140009 },
            { FlycloakType::TheForest, 140010 },
            { FlycloakType::StarlitFeast, 140011 },
        };

        config::Field<TranslatedHotkey> f_Enabled;
        config::Field<config::Enum<FlycloakType>> f_FlycloakType;

        uint32_t GetFlycloakType();

        static FlycloakModifier& GetInstance();
        const FeatureGUIInfo& GetGUIInfo() const override;
        void DrawMain() override;
        bool NeedStatusDraw() const override;
        void DrawStatus() override;

    private:
        FlycloakModifier();
        void OnEntityAppear(app::Proto_SceneEntityAppearNotify* notify);
    };
}