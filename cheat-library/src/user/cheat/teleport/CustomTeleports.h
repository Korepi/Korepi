#pragma once

#include <il2cpp-appdata.h>

#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/Config.h>
#include <cheat-base/thread-safe.h>
#include <set>

namespace cheat::feature
{
    class Teleport
    {
        public:
            Teleport(std::string name, app::Vector3 position, std::string description)
            {
                this->name = name;
                this->position = position;
                this->description = description;
            }
            std::string name;
            app::Vector3 position;
            std::string description;
    };
    
    class CustomTeleports : public Feature
    {
    public:
        config::Field<TranslatedHotkey> f_Enabled;
        config::Field<TranslatedHotkey> f_Interpolate;
        config::Field<TranslatedHotkey> f_Auto;
        config::Field<float> f_Speed;
        config::Field<Hotkey> f_Next;
        config::Field<Hotkey> f_Previous;
        config::Field<int> f_DelayTime;
        static CustomTeleports& GetInstance();
        const FeatureGUIInfo& GetGUIInfo() const override;

        void CheckFolder();
        bool ValidateTeleport(std::string name);
        Teleport Teleport_(std::string name, app::Vector3 position, std::string description);
        void SerializeTeleport(Teleport t);
        void ReloadTeleports();
        std::optional<Teleport> SerializeFromJson(std::string json, bool fromfile);


		void DrawMain() override;
        virtual bool NeedStatusDraw() const override;
        void DrawStatus() override;
        void OnGameUpdate();
        
        std::vector<Teleport> Teleports;
        std::filesystem::path dir;

    private:
        std::set<uint64_t> checkedIndices;
        std::set<uint64_t> searchIndices;
        bool selectedByClick = false;
        std::optional<size_t> selectedIndex;
        std::string selectedName;
        std::string selectedIndexName;
        SafeValue<int64_t> nextTime;
        CustomTeleports();
        void TeleportTo(app::Vector3 position, bool shouldInterpolate);
        void OnTeleportKeyPressed(bool next);
        void OnPrevious();
        void OnNext();
        void UpdateIndexName();
    };

}
