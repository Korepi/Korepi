#include "pch-il2cpp.h"
#include "EmotionChanger.h"
#include "AnimationChanger.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
    // Most of this is copy from AnimationChanger.cpp, plagiarism go brrrrr
    std::vector<std::string> phonemes;
    std::vector<std::string> emotions;

    EmotionChanger::EmotionChanger() : Feature(),
        NFP(f_Enabled, "Visuals::EmotionChanger", "Emotion Changer", false),
        NF(f_Phonemes, "Visuals::EmotionChanger", "P_None"),
        NF(f_Emotions, "Visuals::EmotionChanger", "Normal"),
        NF(f_ApplyKey, "Visuals::EmotionChanger", Hotkey('Y')),
        NF(f_ResetKey, "Visuals::EmotionChanger", Hotkey('R'))
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(EmotionChanger::OnGameUpdate);
    }

    const FeatureGUIInfo& EmotionChanger::GetGUIInfo() const
    {
        TRANSLATED_GROUP_INFO("Emotion Changer", "Visuals");
        return info;
    }

    void EmotionChanger::DrawMain()
    {
        ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Changes active character's emotion."));
        if (f_Enabled->enabled())
        {
            if (ImGui::BeginCombo(_TR("Phonemes"), f_Phonemes.value().c_str()))
            {
                for (auto& phoneme : phonemes)
                {
                    if (phoneme.empty())
                        phoneme = "None";
                    bool is_selected = (f_Phonemes.value().c_str() == phoneme);
                    if (ImGui::Selectable(phoneme.c_str(), is_selected))
                        f_Phonemes.value() = phoneme;

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo(_TR("Emotions") /*idk*/, f_Emotions.value().c_str()))
            {
                for (auto& emotion : emotions)
                {
					if (emotion.empty())
                        emotion = "None";
                    bool is_selected = (f_Emotions.value().c_str() == emotion);
                    if (ImGui::Selectable(emotion.c_str(), is_selected))
                        f_Emotions.value() = emotion;

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ConfigWidget(_TR("Apply Key"), f_ApplyKey, true);
            ConfigWidget(_TR("Reset Key"), f_ResetKey, true);
        }
    }

    bool EmotionChanger::NeedStatusDraw() const
    {
        return f_Enabled->enabled();
    }

    void EmotionChanger::DrawStatus()
    {
        ImGui::Text(_TR("Emotion Changer"));
    }

    EmotionChanger& EmotionChanger::GetInstance()
    {
        static EmotionChanger instance;
        return instance;
    }

    void EmotionChanger::OnGameUpdate()
    {
        auto& animationChanger = AnimationChanger::GetInstance();

        if (!f_Enabled->enabled())
            return;

        auto& manager = game::EntityManager::instance();
        auto avatarName = manager.avatar()->name();

        auto gameObj = app::GameObject_Find(string_to_il2cppi("EntityRoot/AvatarRoot/" + avatarName.substr(0, avatarName.find_first_of(" ")) + "/OffsetDummy/" + avatarName.substr(0, avatarName.find_first_of("("))), nullptr);
        if (gameObj == nullptr)
            return;

        auto emoComponent = app::GameObject_GetComponentByName(gameObj, string_to_il2cppi("EmoSync"), nullptr);
        if (emoComponent == nullptr)
            return;

        auto phonemesArray = reinterpret_cast<app::EmoSync*>(emoComponent)->fields.phonemes->fields._items;
        auto emotionsArray = reinterpret_cast<app::EmoSync*>(emoComponent)->fields.emotions->fields._items;

        static bool isFull = false;
        for (size_t i = 0; i < phonemesArray->max_length && !isFull; i++)
        {
            if (phonemes.size() == phonemesArray->max_length)
            {
                std::sort(phonemes.begin(), phonemes.end());
                phonemes.erase(unique(phonemes.begin(), phonemes.end()), phonemes.end());
                isFull = true;
                continue;
            }

            phonemes.push_back(il2cppi_to_string(phonemesArray->vector[i]->fields.name).c_str());
        }

        for (size_t i = 0; i < emotionsArray->max_length && !isFull; i++)
        {
            if (emotions.size() == emotionsArray->max_length)
            {
                std::sort(emotions.begin(), emotions.end());
                emotions.erase(unique(emotions.begin(), emotions.end()), emotions.end());
                isFull = true;
                continue;
            }

            emotions.push_back(il2cppi_to_string(emotionsArray->vector[i]->fields.name).c_str());
        }

        if (!animationChanger.onEntityAppear && isFull)
        {
            emotions.clear();
            phonemes.clear();
            isFull = false;
            animationChanger.onEntityAppear = true;
        }

        if (f_ApplyKey.value().IsPressed())
        {
            app::EmoSync_SetPhoneme(reinterpret_cast<app::EmoSync*>(emoComponent), string_to_il2cppi(f_Phonemes.value()), 0.0F, nullptr);
            app::EmoSync_SetEmotion(reinterpret_cast<app::EmoSync*>(emoComponent), string_to_il2cppi(f_Emotions.value()), 0.0F, nullptr);
            app::EmoSync_EmoFinish(reinterpret_cast<app::EmoSync*>(emoComponent), nullptr);
            app::EmoSync_PhoFinish(reinterpret_cast<app::EmoSync*>(emoComponent), nullptr);
        }

        if (f_ResetKey.value().IsPressed())
        {
            app::EmoSync_ResetEmotion(reinterpret_cast<app::EmoSync*>(emoComponent), false, nullptr);
            app::EmoSync_ResetPhoneme(reinterpret_cast<app::EmoSync*>(emoComponent), false, nullptr);
            app::EmoSync_EmoFinish(reinterpret_cast<app::EmoSync*>(emoComponent), nullptr);
            app::EmoSync_PhoFinish(reinterpret_cast<app::EmoSync*>(emoComponent), nullptr);
        }
    }
}