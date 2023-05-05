#include "pch-il2cpp.h"
#include "ProfileChanger.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/util.h>
#include <misc/cpp/imgui_stdlib.h>
#include <fstream>
 

namespace cheat::feature
{
    namespace GameObject {
        app::GameObject* WaterMark = nullptr;
    }
    
    namespace Components {
        app::Component_1* WaterMark = nullptr;

        app::Texture2D* CardTexture = nullptr;
        app::Texture2D* AvatarTexture = nullptr;
        app::Sprite* CardSprite = nullptr;
        app::Sprite* AvatarSprite = nullptr;
        app::Rect RectCard;
        app::Rect RectAvatar;
    }

    // Profile Page
    app::Button_1* ProfilePage(app::MonoInLevelPlayerProfilePage* __this, MethodInfo* method);

    // Edit Player Info Page 
    static void ProfileEditPage(app::MonoFriendInformationDialog* __this, app::Sprite* value, MethodInfo* method);

    ProfileChanger::ProfileChanger() : Feature(),
        NFP(f_Enabled, "Visuals::ProfileChanger", "Profile changer", false),
        NF(f_UID, "Visuals::ProfileChanger", false),
        NF(f_UIDWaterMarkPrefix, "Visuals::ProfileChanger", false),
        NF(f_UIDsize, "Visuals::ProfileChanger", 14),
        NF(f_UIDpos_x, "Visuals::ProfileChanger", static_cast<float>(app::Screen_get_width(nullptr)* 0.96875)),
        NF(f_UIDpos_y, "Visuals::ProfileChanger", 0),
        NF(f_NickName, "Visuals::ProfileChanger", false),
        NF(f_Level, "Visuals::ProfileChanger", false),
        NFP(f_Exp, "Visuals::ProfileChanger", "Exp", false),
        NF(f_CurExp, "Visuals::ProfileChanger", 1),
        NF(f_MaxExp, "Visuals::ProfileChanger", 1),
        NFP(f_ExpBar, "Visuals::ProfileChanger", "Exp bar", false),
        NF(f_ExpBarValue, "Visuals::ProfileChanger", 20.0f),
        NF(f_WorldLevel, "Visuals::ProfileChanger", false),
        NF(f_Avatar, "Visuals::ProfileChanger", false),
        NF(f_Card, "Visuals::ProfileChanger", false),
        toBeUpdate(), nextUpdate(0)
    {
        HookManager::install(app::ProfilePage, ProfilePage);
        HookManager::install(app::ProfileEditPage, ProfileEditPage);
        events::GameUpdateEvent += MY_METHOD_HANDLER(ProfileChanger::OnGameUpdate);
    }

    const FeatureGUIInfo& ProfileChanger::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Custom Profile", "Visuals");
        return info;
    }

    void ProfileChanger::DrawMain()
    {
        ConfigWidget(_TR("Custom Profile"), f_Enabled, _TR("Custom Profile."));
        ConfigWidget(_TR("UID"), f_UID, _TR("Changes the UID visually."));
        ConfigWidget(_TR("Append \"UID:\" prefix on the water-mark"), f_UIDWaterMarkPrefix);
        ConfigWidget(_TR("UID size"), f_UIDsize, 0, 1, 500, _TR("Set UID size"));
        ConfigWidget(_TR("UID Pos X"), f_UIDpos_x, 1.f, 1.f, static_cast<float>(app::Screen_get_width(nullptr)), _TR("Set UID position X"));
        ConfigWidget(_TR("UID Pos y"), f_UIDpos_y, 1.f, 0, static_cast<float>(app::Screen_get_height(nullptr)), _TR("Set UID position y"));
        ConfigWidget(_TR("NickName"), f_NickName, _TR("Changes the nickname visually."));
        ConfigWidget(_TR("Level"), f_Level, _TR("Changes the level visually."));
        ConfigWidget(_TR("Exp"), f_Exp, _TR("Changes the exp visually."));
        
        if (f_Exp->enabled()) 
        {
            ConfigWidget(_TR("CurExp"), f_CurExp, 1, 2, INT32_MAX, _TR("Changes the ExpBar visually."));
            ConfigWidget(_TR("MaxExp"), f_MaxExp, 1, 2, INT32_MAX, _TR("Changes the ExpBar visually."));
            ConfigWidget(_TR("ExpBar"), f_ExpBar, _TR("Changes the ExpBar visually."));
            
            if (f_ExpBar->enabled())
                ConfigWidget(_TR("ExpBarValue"), f_ExpBarValue, 1, 2, 100, _TR("Changes the ExpBar visually."));
        }   

        ConfigWidget(_TR("World level"), f_WorldLevel, _TR("Changes the world-level visually."));
        ConfigWidget(_TR("Avatar image"), f_Avatar, _TR("Changes the Avatar Image visually.\nNote the size of the picture must be: 256x256.\nExample path: C:\\Avatars.png"));
        ConfigWidget(_TR("Card image"), f_Card, _TR("Changes the Card visually.\nNote the size of the picture must be: 840x400.\nExample path: C:\\Card.png"));
    }

    bool ProfileChanger::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void ProfileChanger::DrawStatus()
    {
        ImGui::Text(_TR("Custom Profile"));
    }

    ProfileChanger& ProfileChanger::GetInstance()
    {
        static ProfileChanger instance;
        return instance;
    }

    void ProfileChanger::OnGameUpdate()
    {
        if (!f_Enabled->enabled() || !f_UID->enabled())
            return;

        auto currentTime = util::GetCurrentTimeMillisec();
        if (currentTime < nextUpdate)
            return;

        if (f_UID->enabled()) 
        {
            if (GameObject::WaterMark == nullptr)
                GameObject::WaterMark = app::GameObject_Find(string_to_il2cppi("/BetaWatermarkCanvas(Clone)/Panel/TxtUID"), nullptr);
            
            if (GameObject::WaterMark != nullptr && Components::WaterMark == nullptr)
                Components::WaterMark = app::GameObject_GetComponentByName(GameObject::WaterMark, string_to_il2cppi("Text"), nullptr);

            if (Components::WaterMark != nullptr)
                app::Text_set_text(reinterpret_cast<app::Text*>(Components::WaterMark), string_to_il2cppi(f_UID->value().empty() ? "" : std::string((f_UIDWaterMarkPrefix ? "UID: " : "") + f_UID.value().value())), nullptr);

            auto transformWatermark = app::GameObject_get_transform(GameObject::WaterMark, nullptr);
            if (transformWatermark)
            {
                app::Vector3 uidPos = { f_UIDpos_x, f_UIDpos_y, 0 };
                app::Text_set_alignment(reinterpret_cast<app::Text*>(Components::WaterMark), app::TextAnchor__Enum::LowerRight, nullptr);
                app::Text_set_horizontalOverflow(reinterpret_cast<app::Text*>(Components::WaterMark), app::HorizontalWrapMode__Enum::Overflow, nullptr);
                app::Text_set_verticalOverflow(reinterpret_cast<app::Text*>(Components::WaterMark), app::VerticalWrapMode__Enum::Overflow, nullptr);
                app::Text_set_resizeTextForBestFit(reinterpret_cast<app::Text*>(Components::WaterMark), false, nullptr);
                app::Text_set_fontSize(reinterpret_cast<app::Text*>(Components::WaterMark), f_UIDsize, nullptr);
                app::Transform_set_position(transformWatermark, uidPos, nullptr);
            }
        }
           
        nextUpdate = currentTime + (int)f_DelayUpdate;
    }

    bool ProfileChanger::CheckFile(const std::string& Filename) {
        struct stat buffer;
        return (stat(Filename.c_str(), &buffer) == 0);
    }

    app::Button_1* ProfilePage(app::MonoInLevelPlayerProfilePage* __this, MethodInfo* method)
    {
        auto& profile = ProfileChanger::GetInstance();

        if (profile.f_Enabled->enabled()) {
           
            if (profile.f_UID->enabled())
                app::Text_set_text(__this->fields._playerID, string_to_il2cppi(profile.f_UID->value()), nullptr);
                
            if (profile.f_Level->enabled())
                app::Text_set_text(__this->fields._playerLv, string_to_il2cppi(profile.f_Level->value()), nullptr);
 
            if (profile.f_Exp->enabled()) 
            {
                std::string CurExpStr = std::to_string(profile.f_CurExp);
                std::string MaxExpStr = std::to_string(profile.f_MaxExp);
                app::Text_set_text(__this->fields._playerExp, string_to_il2cppi(CurExpStr + "/" + MaxExpStr), nullptr);

                if (profile.f_ExpBar->enabled()) 
                {
                    app::Slider_1_set_minValue(__this->fields._playerExpSlider, 1, nullptr);
                    app::Slider_1_set_maxValue(__this->fields._playerExpSlider, 100, nullptr);
                    app::Slider_1_set_value(__this->fields._playerExpSlider, profile.f_ExpBarValue, nullptr);
                }
            }

            if (profile.f_WorldLevel->enabled())
                app::Text_set_text(__this->fields._playerWorldLv, string_to_il2cppi(profile.f_WorldLevel->value()), nullptr);

            if (profile.f_NickName->enabled())
            {
                auto playerModule = GET_SINGLETON(MoleMole_PlayerModule);
                if (playerModule != nullptr && playerModule->fields._accountData_k__BackingField != nullptr) {
                    auto& accountData = playerModule->fields._accountData_k__BackingField->fields;
                    accountData.nickName = string_to_il2cppi(profile.f_NickName->value());
                }
            }
          
            // Card Name png size 840x400
            if (profile.f_Card->enabled())
            {
                if (profile.CheckFile(profile.f_Card->value())) 
                {
                    Components::CardTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(profile.f_Card->value()), 100, false, false, false, nullptr);           
                  
                    // If you don't do this check, then the UI will break after teleportation, I'm just too lazy to set up Rect manually
                    if (Components::RectCard.m_Width == 0) 
                        Components::RectCard = app::Sprite_get_rect(__this->fields._nameCardPic->fields.m_Sprite, nullptr);
                    
                    app::Vector2 Vec2 = { 100, 100 };
                    Components::CardSprite = app::Sprite_Create(Components::CardTexture, Components::RectCard, Vec2, 1, nullptr);
                    __this->fields._nameCardPic->fields.m_OverrideSprite = Components::CardSprite;  
                }
                else {
                    std::cout << "Card Image: \n" << "not found" << std::endl;
                }
            }
            // Avatar png size 256x256
            if (profile.f_Avatar->enabled())
            {
                if (profile.CheckFile(profile.f_Avatar->value())) 
                {
                    Components::AvatarTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(profile.f_Avatar->value()), 100, false, false, false, nullptr);
                    
                    // If you don't do this check, then the UI will break after teleportation, I'm just too lazy to set up Rect manually
                    if (Components::RectAvatar.m_Width == 0)
                        Components::RectAvatar = app::Sprite_get_rect(__this->fields.playerIconImage->fields.m_Sprite, nullptr);
                    
                    app::Vector2 Vec2Avatar = { 128, 128 };
                    Components::AvatarSprite = app::Sprite_Create(Components::AvatarTexture, Components::RectAvatar, Vec2Avatar, 1, nullptr);
                    __this->fields.playerIconImage->fields.m_OverrideSprite = Components::AvatarSprite;
                }
                else 
                {
                    std::cout << "Avatar Image: \n" << "not found" << std::endl;
                }
            }
        }
        return CALL_ORIGIN(ProfilePage, __this, method);
    }

    static void ProfileEditPage(app::MonoFriendInformationDialog* __this, app::Sprite* value, MethodInfo* method) {
        auto& profile = ProfileChanger::GetInstance();

        auto accountUid = std::to_string(game::GetAccountData()->fields.userId);
        auto profileUid = il2cppi_to_string(app::Text_get_text(__this->fields._playerUID, nullptr));
        bool isMe = accountUid.compare(profileUid) == 0;

        if (profile.f_Enabled->enabled() && isMe)
        {
            if (profile.f_UID->enabled())
                __this->fields._playerUID->fields.m_Text = string_to_il2cppi(profile.f_UID->value());
 
            if (profile.f_Level->enabled())
                __this->fields._playerLevel->fields.m_Text = string_to_il2cppi(profile.f_Level->value());

            if (profile.f_WorldLevel->enabled())
                __this->fields._worldLevel->fields.m_Text = string_to_il2cppi(profile.f_WorldLevel->value());

            // Card Name png size 840x400

            if (profile.f_Card->enabled()) 
            {
                if (profile.CheckFile(profile.f_Card->value())) 
                {
                    Components::CardTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(profile.f_Card->value()), 100, false, false, false, nullptr);
                    // If you don't do this check, then the UI will break after teleportation, I'm just too lazy to set up Rect manually
                    if (Components::RectCard.m_Width == 0)
                        Components::RectCard = app::Sprite_get_rect(__this->fields._cardImg->fields.m_Sprite, nullptr);
                    app::Vector2 Vec2 = { 100, 100 };
                    Components::CardSprite = app::Sprite_Create(Components::CardTexture, Components::RectCard, Vec2, 1, nullptr);
                    __this->fields._cardImg->fields.m_OverrideSprite = Components::CardSprite;
                }
                else {
                    std::cout << "Card Image: \n" << "not found" << std::endl;
                }
            }

            // Avatar png size 256x256
            if (profile.f_Avatar->enabled()) 
            {
                if (profile.CheckFile(profile.f_Avatar->value())) 
                {
                    Components::AvatarTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(profile.f_Avatar->value()), 100, false, false, false, nullptr);
                    if (Components::RectAvatar.m_Width == 0)
                        Components::RectAvatar = app::Sprite_get_rect(__this->fields._icon->fields.m_Sprite, nullptr);
                    app::Vector2 Vec2Avatar = { 128, 128 };
                    Components::AvatarSprite = app::Sprite_Create(Components::AvatarTexture, Components::RectAvatar, Vec2Avatar, 1, nullptr);
                    __this->fields._icon->fields.m_OverrideSprite = Components::AvatarSprite;
                }
                else 
                {
                    std::cout << "Card Image: \n" << "not found" << std::endl;
                }
            }      
        }

        return CALL_ORIGIN(ProfileEditPage, __this, value, method);
    }
}