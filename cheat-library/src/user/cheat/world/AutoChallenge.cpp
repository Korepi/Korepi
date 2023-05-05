#include "pch-il2cpp.h"
#include "AutoChallenge.h"

#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/filters.h>
#include <cheat/esp/ESP.h>
#include <misc/cpp/imgui_stdlib.h>

namespace cheat::feature
{
    const double eps = 1e-8;
#define Equ(a, b) (fabs((a)-(b))<(eps))

	AutoChallenge::AutoChallenge() : Feature(),
		NFP(f_Enabled, "AutoChallenge", "Auto Challenge", false),
		NFP(f_BombDestroy, "AutoChallenge", "Destroy Bomb", false),
		NFP(f_CustomChallange, "AutoChallenge", "Cuntom Challenge", false),
		NF(f_Delay, "AutoChallenge", 1000),
		NF(f_Range, "AutoChallenge", 20.f),
		NFEX(f_CustomJson, "CustomCollection", "AutoChallenge", std::vector<nlohmann::json>(), false),
		m_GadgetId(0),
		m_DisplayName("")
	{
		InitCustomCollections();
		events::GameUpdateEvent += MY_METHOD_HANDLER(AutoChallenge::OnGameUpdate);
	}

	const FeatureGUIInfo& AutoChallenge::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Auto Challenge", "World");
		return info;
	}

	void AutoChallenge::DrawMain()
	{
		ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Auto collect time challenge item"));
		ImGui::SameLine();
		ConfigWidget(_TR("Destroy Bomb"), f_BombDestroy, _TR("Auto destroy bombbarrel"));
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("I haven't tested how high the risk is"));
		ImGui::SetNextItemWidth(200.f);
		ConfigWidget(_TR("Range"), f_Range, 0.1f, 0.f, 300.f, _TR("Collect range."));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.f);
		ConfigWidget(_TR("Delay"), f_Delay, 1, 0, 2000, _TR("Collect delay."));
		ConfigWidget(_TR("Enable Custom Challange"), f_CustomChallange, _TR("Get gadget id from entity manager."));
		ImGui::PushID("CustomChallange");
		if (ImGui::BeginGroupPanel(_TR("Custom Challange")))
		{
			ImGui::SetNextItemWidth(250);
			ImGui::InputText(_TR("Display Name"), &m_DisplayName);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(250);
			ImGui::InputInt(_TR("Gadget Id"), &m_GadgetId, 0, 0, 1);
			ImGui::SameLine();
			if (ImGui::Button(Translator::RuntimeTranslate("Add").c_str()))
				AddCustomChallange(m_GadgetId, m_DisplayName, true);

			ImGui::Spacing();
			DrawCustomChallangeTable();
		}
		ImGui::EndGroupPanel();

		ImGui::PopID();
	}

	bool AutoChallenge::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}

	void AutoChallenge::DrawStatus()
	{
		ImGui::Text("%s [%.01fm]", _TR("Challenge"), f_Range.value());
	}

	AutoChallenge& AutoChallenge::GetInstance()
	{
		static AutoChallenge instance;
		return instance;
	}

	void AutoChallenge::OnGameUpdate()
	{
		static uint64_t lastTime = 0;
		auto timestamp = app::MoleMole_TimeUtil_get_LocalNowMsTimeStamp(nullptr);

		if (!f_Enabled->enabled() || lastTime + f_Delay > timestamp)
			return;

		auto& entityManager = game::EntityManager::instance();
		auto& esp = ESP::GetInstance();
		auto avatarEntity = entityManager.avatar();

		for (auto& entity : entityManager.entities())
		{
			if (avatarEntity->distance(entity) > f_Range)
				continue;
			if (m_Collections.find(esp.m_Gadgets[entity->runtimeID()].gadgetId) != m_Collections.end() || \
				(f_CustomChallange->enabled() &&
					std::any_of(m_CustomCollections.begin(), m_CustomCollections.end(), [&](const auto& collection) {
						return collection.first == esp.m_Gadgets[entity->runtimeID()].gadgetId;
						}))
				)
				entity->setRelativePosition(avatarEntity->relativePosition());
		}
	}

	void AutoChallenge::InitCustomCollections()
	{
		if (f_CustomJson.value().is_null() || !f_CustomJson.value().is_array()) {
			LOG_WARNING("Failed to load custom collection data.");
			return;
		}

		auto jItems = f_CustomJson.value().get<std::vector<nlohmann::json>>();
		for (auto& jObject : jItems)
		{
			try
			{
				AddCustomChallange(jObject["gadgetId"], jObject["displayName"], false);
			}
			catch (const std::exception& e)
			{
				LOG_WARNING("Failed to initialize custom collection item.\n%s\n\nJSON dump:\n%s", e.what(), jObject.dump(4).c_str());
				continue;
			}
		}
	}

	void AutoChallenge::AddCustomChallange(uint32_t gadgetId, std::string displayName, bool convertToJson)
	{
		m_CustomCollections.push_back({ gadgetId, displayName });
		m_GadgetId = 0;
		m_DisplayName = "";
		if (convertToJson)
			SaveCustomCollections();
	}

	void AutoChallenge::SaveCustomCollections()
	{
		std::vector<nlohmann::json> jItems = {};
		for (auto& collection : m_CustomCollections)
		{
			jItems.push_back({
				{ "gadgetId", collection.first },
				{ "displayName", collection.second } });
		}

		f_CustomJson = config::converters::ToJson(jItems);
	}

	void AutoChallenge::DrawCustomChallangeTable()
	{
		const float clipSize = static_cast<float>(min(m_CustomCollections.size(), 15) + 1); // Number of rows in table as initial view. Past this is scrollbar territory.
		static ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;

		static int rowToDelete = -1;
		if (ImGui::BeginTable("Collections", 3, flags, ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * clipSize + 5.f), 5.f))
		{
			ImGui::TableSetupColumn(_TR("Display Name"), ImGuiTableColumnFlags_WidthStretch, 0.f, 0);
			ImGui::TableSetupColumn(_TR("Gadget Id"), ImGuiTableColumnFlags_WidthStretch, 0.f, 1);
			ImGui::TableSetupColumn(_TR("Actions"), ImGuiTableColumnFlags_WidthFixed, 0.f, 2);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(m_CustomCollections.size()));
			while (clipper.Step())
				for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
				{
					auto& collection = m_CustomCollections[row_n];

					ImGui::PushID((collection.second + ":" + std::to_string(collection.first)).c_str());
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGui::Text("%s", collection.second.c_str());
					ImGui::TableNextColumn();

					ImGui::Text("%u", collection.first);
					ImGui::TableNextColumn();

					if (ImGui::Button(_TR("Delete")))
						rowToDelete = row_n;

					ImGui::PopID();
				}
			ImGui::EndTable();
		}

		if (rowToDelete > -1)
		{
			m_CustomCollections.erase(m_CustomCollections.begin() + rowToDelete);
			rowToDelete = -1;
			SaveCustomCollections();
		}
	}
}