#include "pch-il2cpp.h"
#include "About.h"

#include <cheat/game/util.h>
#include <cheat/events.h>
#include <regex>

namespace cheat::feature
{
    About::About() : Feature(),
        NF(f_IsFirstTime, "About", true)
    {
        std::string syslang = setlocale(LC_ALL, "");
        if (syslang.starts_with("Arabic_"))
            lang = "ArabicW";
        else if (syslang.starts_with("Chinese (Simplified)_"))
            lang = "ZHCN";
        else if (syslang.starts_with("Chinese (Traditional)_"))
            lang = "ZHTW";
        else if (syslang.starts_with("Filipino_"))
            lang = "FilipinoW";
        else if (syslang.starts_with("Japanese_"))
            lang = "JapaneseW";
        else if (syslang.starts_with("Malay_"))
            lang = "MalayW";
        else if (syslang.starts_with("Portuguese_"))
            lang = "PortugueseW";
        else if (syslang.starts_with("Russian_"))
            lang = "RussianW";
        else if (syslang.starts_with("Vietnamese_"))
            lang = "VietnameseW";
        else
            lang = "EnglishW";

        width = app::Screen_get_width(nullptr);
        height = app::Screen_get_height(nullptr);

        if ((float)width / (float)height > 2) { width_picture = width * 0.305; height_picture = height * 0.13; }          // 21:9
        else if ((float)width / (float)height > 1.6) { width_picture = width * 0.4; height_picture = height * 0.13; }     // 16:9
        else if ((float)width / (float)height > 1.4) { width_picture = width * 0.4; height_picture = height * 0.117; }    // 16:10
        else { width_picture = width * 0.4; height_picture = height * 0.0975; }                                           // 4:3

        timer = util::GetCurrentTimeMillisec();

        events::GameUpdateEvent += MY_METHOD_HANDLER(About::OnGameUpdate);
    }
    const FeatureGUIInfo& About::GetGUIInfo() const
    {
        TRANSLATED_MODULE_INFO("About");
        return info;
    }

    void About::DrawMain()
    {
        std::optional<ImageLoader::GIFData*> gif = ImageLoader::GetGIF("ANIM_KOREPIBOUNCE");
        if (gif)
        {
            gif.value()->render(ImVec2(ImGui::GetWindowSize().x / 3.5f, ImGui::GetWindowSize().y / 2.5f));
        }

        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetWindowSize().x);

        //ImGui::TextColored(ImColor(28, 202, 214, 255), _TR("Korepi github:"));
        //TextURL(_TR("GitHub link"), "https://github.com/Papaya-Group/Korepi-GC/", true, false);

        ImGui::TextColored(ImColor(28, 202, 214, 255), _TR("Korepi discord:"));
        TextURL(_TR("Discord invite link"), "https://discord.gg/8UZbDtEvrW", true, false);

        ImGui::Text(_TR("Founder:"));
        ImGui::SameLine();
        ImGui::TextColored(ImColor(0, 102, 255, 255), "Callow");

        ImGui::Text(_TR("Main developer and updater:"));
        ImGui::SameLine();
        ImGui::TextColored(ImColor(0, 102, 255, 255), "Strigger");

        ImGui::Text(_TR("Main contributors:"));
        ImGui::TextColored(ImColor(0, 102, 255, 255), "Taiga74164, RyujinZX, WitchGod, m0nkrel, harlanx, andiabrudan, hellomykami, NctimeAza, FawazTakhji, RedDango, RainAfterDark");

        //ImGui::Text(_TR("Full contributor list:"));
        //TextURL(_TR("GitHub link"), "https://github.com/Papaya-Group/Korepi-GC/graphs/contributors", true, false);

        ImGui::Text(_TR("Translators list:"));
        ImGui::Text(_TR("  Russian:"));
        ImGui::TextColored(ImColor(0, 102, 255, 255), "    - phenacemide#2436");
        ImGui::TextColored(ImColor(0, 102, 255, 255), "    - AMOGUSS#0530");
        ImGui::Text(_TR("  Chinese:"));
        ImGui::TextColored(ImColor(0, 102, 255, 255), "    - Word.exe#1377");
        ImGui::TextColored(ImColor(0, 102, 255, 255), "    - ShinaKaria#9760");
        ImGui::TextColored(ImColor(0, 102, 255, 255), "    - black_zero#3327");
        ImGui::TextColored(ImColor(0, 102, 255, 255), "    - (ZH-TW) xTaiwanPingLord#9960");
        ImGui::TextColored(ImColor(0, 102, 255, 255), "    - (Proofread) inkay#5122");
        ImGui::PopTextWrapPos();
    }

    void About::OnGameUpdate()
    {
        UPDATE_DELAY(100);
        if (show)
        {
            if (int(util::GetCurrentTimeMillisec() - timer) > (int)60000)
                show = false;
        }

        if (!m_IsScamWarningShowed)// && f_IsFirstTime)
            ShowInGameScamWarning();
    }

    About& About::GetInstance()
    {
        static About instance;
        return instance;
    }

    void About::ShowInGameScamWarning()
    {
        if (!game::IsInGame())
            return;
        auto warning = GetScamWarningW();
        game::ShowInGameGeneralDialog(warning.first, warning.second, app::GeneralDialogContext_GeneralDialogType__Enum::SINGLE_BUTTON);
        //game::ShowInGameInfoMessage(warning.second, true, app::UIShowPriority__Enum::None);
        m_IsScamWarningShowed = true;
        //f_IsFirstTime = false;
    }

    std::pair<std::wstring, std::wstring> About::GetScamWarningW()
    {
        auto lang = app::Application_get_systemLanguage(nullptr);

        switch (lang)
        {
        case app::SystemLanguage__Enum::ChineseSimplified:
            return std::make_pair(L"<i>欺诈警告！</i>", L"这是一个<color=#a80000ff>公开、免费使用</color>的辅助。\n如果你是付费购买的，那么你被骗了。\n<size=20>更多相关消息，请查看菜单中的\"关于\"</size>");
        case app::SystemLanguage__Enum::ChineseTraditional:
            return std::make_pair(L"<i>欺詐警告！</i>", L"這是一個<color=#a80000ff>公開、免費使用</color>的輔助。\n如果你是付費購買的，那麼你被騙了。\n<size=20>更多相關消息，請查看菜單中的\"關於\"</size>");
        case app::SystemLanguage__Enum::Japanese:
            return std::make_pair(L"<i>詐欺警告！</i>", L"このチートは<color=#a80000ff>無料で使用できますが</color>、\n購入した場合は詐欺に遭っています。\n<size=20>詳細については「About」を確認してください</size>");
        case app::SystemLanguage__Enum::Vietnamese:
            return std::make_pair(L"<i>Cảnh báo lừa đảo!</i>", L"Đây là <color=#a80000ff>phần mềm miễn phí</color>. \nNếu bạn trả tiền cho nó, bạn đã bị lừa đảo. \n<size=20>Để biết thêm thông tin chi tiết, vui lòng kiểm tra phần \"About\" trong Menu</size>");
        case app::SystemLanguage__Enum::French:
            return std::make_pair(L"<i>Alerte Anarque!</i>", L"Ce cheat est entièrement <color=#a80000ff>Gratuit d’utilisation</color>.\nSi vous l’avez acheté, vous avez subis une arnaque.\n<size=20>Pour plus d’informations lisez le panneau \"À Propos\".</size>");
        case app::SystemLanguage__Enum::Spanish:
            return std::make_pair(L"<i>¡Advertencia de estafa!</i>", L"Este truco es <color=#a80000ff>Gratuito</color>.\nSi lo has comprado, te han estafado.\n<size=20>Para más información consulta la pestaña \"Acerca\" de</size>");
        case app::SystemLanguage__Enum::Portuguese:
            return std::make_pair(L"<i>Alerta de scam!</i>", L"Esse cheat é <color=#a80000ff>gratuito</color>. \nSe você pagou por ele, você foi scammado. \n<size=20>Para mais informações, cheque a guia \"About\"</size>");
        case app::SystemLanguage__Enum::Russian:
            return std::make_pair(L"<i>Предупреждение о мошенничестве!</i>", L"Данный чит является <color=#a80000ff>бесплатным</color>. \nEсли вы его купили - поздравляем, вас обманули. \n<size=20>Для получения дополнительной информации смотрите вкладку \"About\"</size>.");
        case app::SystemLanguage__Enum::Ukrainian:
            return std::make_pair(L"<i>Попередження про шахрайство!</i>", L"Цей чит є <color=#a80000ff>безкоштовним</color>. \nЯкщо ви купили його  - вітаємо, вас ошукали. \n<size=20>Для отримання додаткової інформації дивіться вкладку \"About\"</size>.");
        case app::SystemLanguage__Enum::German:
            return std::make_pair(L"<i>Warnung vor Betrug!</i>", L"Dieser Cheat kann <color=#a80000ff>kostenlos heruntergeladen werden</color>.\nWenn Sie ihn gekauft haben, wurden Sie um ihr geld betrogen.\n<size=20>Weitere Informationen finden Sie auf der \"about\" Registerkarte</size>");
        case app::SystemLanguage__Enum::Danish:
            return std::make_pair(L"<i>Svindel Advarsel!</i>", L"Dette snyd er<color=#a80000ff>gratis at bruge</color>.\nHvis du har købt det, er du blevet svindlet.\n<size=20>For mere information tjek \"About\" kanalen</size>");
        default:
            return std::make_pair(L"<i>Scam Warning!</i>", L"This cheat is <color=#a80000ff>Free to use</color>.\nIf you've bought it, you've been scammed.\n<size=20>For more info check the \"About\" tab</size>");
        }
    }

}