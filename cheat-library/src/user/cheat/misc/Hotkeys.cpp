#include "pch-il2cpp.h"
#include "Hotkeys.h"

#include <cheat-base/cheat/fields/TranslatedHotkey.h>
#include <cheat-base/render/gui-util.h>
#include <misc/cpp/imgui_stdlib.h>

namespace cheat::feature 
{
    Hotkeys::Hotkeys() : Feature() { }

    const FeatureGUIInfo& Hotkeys::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("Hotkeys");
        return info;
    }

    void Hotkeys::DrawMain()
    {
        static std::string searchBuffer;
        ImGui::InputText(_TR("Search"), &searchBuffer);

        ImGui::BeginChild("Hotkeys");
        
        std::unordered_map<std::string, std::vector<config::Field<TranslatedHotkey>*>> sections;

        for (auto& field : config::GetFields<TranslatedHotkey>())
        {
            if (!searchBuffer.empty())
            {
                auto name = field->name();
                auto it = std::search(
                    name.begin(), name.end(),
                    searchBuffer.begin(), searchBuffer.end(),
                    [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
                );
                if (it == name.end())
                    continue;
            }

            sections[field.section()].push_back(&field);
        }

        std::vector<config::Field<TranslatedHotkey>*> singleLineSections;
        std::vector<std::vector<config::Field<TranslatedHotkey>*>*> multiLineSections;
        for (auto& [section, fields] : sections)
        {
            if (fields.size() == 1)
                singleLineSections.push_back(fields[0]);
            else
                multiLineSections.push_back(&fields);
        }

        for (auto& field : singleLineSections)
        {
            ConfigWidget(field->value().name().c_str(), *field, nullptr, true);
        }

        for (auto& fields : multiLineSections)
        {
	        if (ImGui::BeginGroupPanel((*fields)[0]->section().c_str(), true))
	        {
		        for (auto& field : *fields)
		        {
                    ConfigWidget(field->value().name().c_str(), *field, nullptr, true);
		        }
	        }
            ImGui::EndGroupPanel();
        }

        ImGui::EndChild();
    }

    Hotkeys& Hotkeys::GetInstance()
    {
        static Hotkeys instance;
        return instance;
    }
}

