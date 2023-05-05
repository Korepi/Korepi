#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature
{
    class SkinModifier : public Feature
    {
    public:
        enum class Skins {
            Qin_SeaBreezeDandelion,
            Qin_GunnhildrsLegacy,
            Barbara_SummertimeSparkle,
            Ningguang_OrchidsEveningGown,
            Keqing_OpulentSplendor,
            Ambor_100Outrider,
            Mona_PactOfStarsandMoon,
            Rosaria_ToTheChurchsFreeSpirit,
            Diluc_RedDeadOfNight,
            Fischl_EinImmernachtstraum,
            Ayaka_SpringbloomMissive,
            Lisa_ASobriquetUnderShade,
        };

        const std::map<uint32_t, std::vector<std::pair<Skins, uint32_t>>> m_AllSkins
        {
            { 10000003, { { Skins::Qin_SeaBreezeDandelion, 200301 }, { Skins::Qin_GunnhildrsLegacy, 200302 } } },
            { 10000014, { { Skins::Barbara_SummertimeSparkle, 201401 } } },
            { 10000027, { { Skins::Ningguang_OrchidsEveningGown, 202701 } } },
            { 10000042, { { Skins::Keqing_OpulentSplendor, 204201 } } },
            { 10000021, { { Skins::Ambor_100Outrider, 202101 } } },
            { 10000041, { { Skins::Mona_PactOfStarsandMoon, 204101 } } },
            { 10000045, { { Skins::Rosaria_ToTheChurchsFreeSpirit, 204501 } } },
            { 10000016, { { Skins::Diluc_RedDeadOfNight, 201601 } } },
            { 10000031, { { Skins::Fischl_EinImmernachtstraum, 203101 } } },
            { 10000002, { { Skins::Ayaka_SpringbloomMissive, 200201 } } },
            { 10000006, { { Skins::Lisa_ASobriquetUnderShade, 200601 } } },
        };

        config::Field<TranslatedHotkey> f_Enabled;
        std::vector<std::pair<config::Field<bool>, Skins>> m_Skins;

        static SkinModifier& GetInstance();
        const FeatureGUIInfo& GetGUIInfo() const override;
        void DrawMain() override;
        bool NeedStatusDraw() const override;
        void DrawStatus() override;

        void InitSkins();

        uint32_t GetSkinId(uint32_t avaterId);
    private:
        SkinModifier();
        void OnEntityAppear(app::Proto_SceneEntityAppearNotify* notify);
    };
}