#include <pch.h>
#include "Language.h"

#include <cheat-base/config/Config.h>
#include <cheat-base/Translator.h>
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/render/gui-util.h>
#include <cheat-base/render/renderer.h>
#include <cheat-base/cheat/CheatManagerBase.h>

#include <fstream>

namespace cheat::feature 
{
    Language::Language() : Feature(),
        NF(f_Language, "Settings::Language", "default")
    {
		Translator::SetLanguage(f_Language);
    }

    const FeatureGUIInfo& Language::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("Settings");
        return info;
    }

	void Language::DrawMain()
	{
        if (ImGui::BeginCombo(_TR("Language"), Translator::GetLanguagePseudo(f_Language).c_str()))
        {
            for (auto& language : Translator::GetLanguages())
            {
                bool is_selected = (language == f_Language.value());
                if (ImGui::Selectable(Translator::GetLanguagePseudo(language).c_str(), is_selected))
                {
                    f_Language = language;
                    Translator::SetLanguage(language);
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

#ifdef _DEBUG
        if (ImGui::Button("Dump translates"))
        {
            auto filepath = util::GetCurrentPath() / "languages.json";
            std::ofstream ofile(filepath);
            ofile << Translator::DumpTranslateConfigTemplate().dump(4);
        }
#endif
    }

    Language& Language::GetInstance()
    {
        static Language instance;
        return instance;
    }
}
