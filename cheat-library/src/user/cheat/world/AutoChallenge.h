#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

namespace cheat::feature 
{

    class AutoChallenge : public Feature
    {
    public:
        std::unordered_map<uint32_t, std::string> m_Collections
        {
            { 70290352, "GeoGlideChampOrb" },
            { 70290501, "DendroGlideChampOrb" },
            { 70290150, "ElecGlideChampOrb" },
            { 70360094, "GlideChampOrb" },
            { 70800268, "GlideChampOrb_For_Activity_WindField" },
            { 70800281, "BigGlideChampOrb" },
            { 70220104, "DreamlandSpirit" }
        };

        std::vector<std::pair<uint32_t, std::string>> m_CustomCollections;
        config::Field<TranslatedHotkey> f_Enabled;
        config::Field<TranslatedHotkey> f_BombDestroy;
        config::Field<TranslatedHotkey> f_CustomChallange;
        config::Field<int> f_Delay;
        config::Field<float> f_Range;
        config::Field<nlohmann::json> f_CustomJson;

        int m_GadgetId;
        std::string m_DisplayName;

        void AddCustomChallange(uint32_t gadgetId, std::string displayName, bool convertToJson);
        void SaveCustomCollections();
        void InitCustomCollections();

        static AutoChallenge& GetInstance();

        void OnGameUpdate();

        const FeatureGUIInfo& GetGUIInfo() const override;
        void DrawMain() override;

        virtual bool NeedStatusDraw() const override;
        void DrawStatus() override;

    private:
        AutoChallenge();
        void DrawCustomChallangeTable();
    };
}

