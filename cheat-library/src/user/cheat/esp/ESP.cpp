#include "pch-il2cpp.h"
#include "ESP.h"

#include <helpers.h>
#include <algorithm>
#include <ranges>
#include <regex>

#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/EntityAppearManager.h>
#include "ESPRender.h"
#include <cheat/game/filters.h>
#include <cheat/game/util.h>
#include <cheat/game/xxHash.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include "cheat-base/cheat/CheatManagerBase.h"

namespace cheat::feature
{
	using namespace xxh;

	ESP::ESP() : Feature(),
		NFP(f_Enabled, "ESP", "ESP", false),

		NF(f_DrawBoxMode, "ESP", DrawMode::Box),
		NF(f_CorneredBoxSize, "ESP", 3.5f),
		NF(f_DrawTracerMode, "ESP", DrawTracerMode::Line),
		NF(f_Fill, "ESP", false),
		NF(f_FillTransparency, "ESP", 0.5f),

		NF(f_ArrowRadius, "ESP", 100.0f),
		NF(f_TracerSize, "ESP", 1.0f),
		NF(f_OutlineThickness, "ESP", 1.0f),
		NF(f_OutlineColor, "ESP", ImColor(0.91f, 0.68f, 0.36f)),
		NF(f_CircleColor, "ESP", ImColor(0.23f, 0.26f, 0.32f)),
		NF(f_ShowArrowIcons, "ESP", true),
		NF(f_ShowHDIcons, "ESP", false),

		NF(f_MiddleScreenTracer, "ESP", false),
		NF(f_DrawDistance, "ESP", false),
		NF(f_DrawName, "ESP", false),
		NF(f_DrawHealth, "ESP", false),
		NF(f_HideCompleted, "ESP", false),

		NF(f_FontSize, "ESP", 12.0f),
		NF(f_FontOutline, "ESP", true),
		NF(f_FontOutlineSize, "ESP", 1.0f),

		NF(f_GlobalFontColor, "ESP", ImColor(255, 255, 255)),
		NF(f_GlobalBoxColor, "ESP", ImColor(255, 255, 255)),
		NF(f_GlobalLineColor, "ESP", ImColor(255, 255, 255)),
		NF(f_GlobalRectColor, "ESP", ImColor(255, 255, 255)),

		NF(f_MinSize, "ESP", 0.5f),
		NF(f_Range, "ESP", 100.0f),

		NFS(f_ShowCustomFiltersWindow, "ESP", false),
		NFEX(f_CustomFilterJson, "CustomFilter", "ESP", std::vector<nlohmann::json>(), false),
		NFS(f_CustomFilterType, "ESP", app::EntityType__Enum_1::None),
		m_CustomFilterUiName({}),
		m_CustomFilterNameToAdd({}),
		m_CustomFilterNames({}),
		m_CustomFilters({}),
		m_CustomFilterInfos({}),
		i_CustomFiltersEditId(-1),
		i_CustomFilterNameEditId(-1),
		m_Search({})
	{
		InstallFilters();
		InstallCustomFilters();

		m_FontContrastColor = ImGui::CalcContrastColor(f_GlobalFontColor);

		::events::KeyUpEvent += MY_METHOD_HANDLER(ESP::OnKeyUp);
	}


	const FeatureGUIInfo& ESP::GetGUIInfo() const
	{
		TRANSLATED_MODULE_INFO("ESP");
		return info;
	}

	void ESP::DrawMain()
	{
		if (ImGui::BeginGroupPanel(_TR("General"), true))
		{
			ConfigWidget(_TR("ESP Enabled"), f_Enabled, _TR("Show filtered object through obstacles."));
			ConfigWidget(_TR("Range (m)"), f_Range, 1.0f, 1.0f, 200.0f);

			ConfigWidget(_TR("Draw Mode"), f_DrawBoxMode, _TR("Select the mode of box drawing."));
			ConfigWidget(_TR("Tracer Mode"), f_DrawTracerMode, _TR("Select the mode of tracer drawing."));
			
			if (f_DrawBoxMode.value() == ESP::DrawMode::CornerBox) {
				ConfigWidget(_TR("Cornered Box Size"), f_CorneredBoxSize, 0.5f, 1.f, 50.f, _TR("Size of cornered box"));
			}

			ConfigWidget(_TR("Fill Box/Rectangle/Arrows"), f_Fill);
			ConfigWidget(_TR("Fill Transparency"), f_FillTransparency, 0.01f, 0.0f, 1.0f, _TR("Transparency of filled part."));
			ConfigWidget(_TR("Middle Screen Tracer"), f_MiddleScreenTracer, _TR("Draw tracer from middle part of the screen."));

			if (f_DrawTracerMode.value() == DrawTracerMode::OffscreenArrows)
			{
				if (ImGui::BeginGroupPanel(_TR("Arrow tracer options"), true))
				{
					ConfigWidget(_TR("Tracer Size"), f_TracerSize, 0.005f, 0.1f, 10.0f, _TR("Size of tracer."));
					ConfigWidget(_TR("Arrow Radius"), f_ArrowRadius, 0.5f, 50.0f, 400.0f, _TR("Radius of arrow."));
					ConfigWidget(_TR("Outline Thickness"), f_OutlineThickness, 0.005f, 1.0f, 10.0f, _TR("Outline thickness of arrow."));

					ConfigWidget(_TR("Outline Color"), f_OutlineColor, _TR("Color of the arrow outline."));
					ConfigWidget(_TR("Circle Color"), f_CircleColor, _TR("Color of the circle icon fill."));

					ConfigWidget(_TR("Show Icons"), f_ShowArrowIcons, _TR("Show arrows with icons."));
					if (f_ShowArrowIcons)
					{
						ImGui::SameLine();
						ConfigWidget(_TR("HD"), f_ShowHDIcons, _TR("Toggle icons to HD format."));
					}
				}
				ImGui::EndGroupPanel();
			}

			ImGui::Spacing();
			ConfigWidget(_TR("Draw Name"), f_DrawName, _TR("Draw name of object."));
			ConfigWidget(_TR("Draw Distance"), f_DrawDistance, _TR("Draw distance of object."));
			ImGui::Spacing();
			ConfigWidget(_TR("Draw Health"), f_DrawHealth, _TR("Draw health of object."));
			ImGui::Spacing();
			ConfigWidget(_TR("Hide completed puzzles"), f_HideCompleted, _TR("Hide completed puzzles (Elemental monuments, Bloatty Floatty, Electro Seelie)"));

			ImGui::Spacing();
			ConfigWidget(_TR("Font Size"), f_FontSize, 1, 1, 100, _TR("Font size of name or distance."));
			ConfigWidget("## Font outline enabled", f_FontOutline); ImGui::SameLine();
			ConfigWidget(_TR("Font outline"), f_FontOutlineSize, 0.001f, 0.0f, 10.0f);

			ImGui::Spacing();
			if (ImGui::BeginGroupPanel(_TR("Global colors"), true))
			{
				if (ConfigWidget(_TR("Font Color"), f_GlobalFontColor, _TR("Color of line, name, or distance text font.")))
					m_FontContrastColor = ImGui::CalcContrastColor(f_GlobalFontColor);

				ConfigWidget(_TR("Box Color"), f_GlobalBoxColor, _TR("Color of box font."));
				ConfigWidget(_TR("Tracer Color"), f_GlobalLineColor, _TR("Color of line font."));
				ConfigWidget(_TR("Rect Color"), f_GlobalRectColor, _TR("Color of rectangle font."));
			}
			ImGui::EndGroupPanel();

			ConfigWidget(_TR("Min. Entity Size"), f_MinSize, 0.05f, 0.1f, 200.0f, _TR("Minimum entity size as measured in-world.\n" \
				"Some entities have either extremely small or no bounds at all.\n" \
				"This parameter helps filter out entities that don't meet this condition."));

			ConfigWidget(_TR("Show custom filters config window"), f_ShowCustomFiltersWindow, _TR("This is for advanced users..."));
		}
		ImGui::EndGroupPanel();

		ImGui::Text(_TR("How to use item filters:\n\tLMB - Toggle visibility\n\tRMB - Open color picker"));
		ImGui::InputText(_TR("Search Filters"), &m_Search);

		ImGui::PushID("Custom filters");
		DrawCustomSection();
		ImGui::PopID();

		for (auto& [section, filters] : m_Sections)
		{
			ImGui::PushID(section.c_str());
			DrawSection(section, filters);
			ImGui::PopID();
		}
	}

	bool ESP::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}

	void ESP::DrawStatus()
	{
		ImGui::Text("%s [%.01fm|%s|%s%s%s%s%s]",
			_TR("ESP"),
			f_Range.value(),
			f_DrawBoxMode.value() == DrawMode::Box ? _TR("Box") : f_DrawBoxMode.value() == DrawMode::Rectangle ? _TR("Rect") : f_DrawBoxMode.value() == DrawMode::CornerBox ? _TR("CornerBox") : _TR("None"),
			f_Fill ? "F" : "",
			f_DrawTracerMode.value() == DrawTracerMode::Line ? "L" : f_DrawTracerMode.value() == DrawTracerMode::OffscreenArrows ? "A" : "",
			f_DrawName ? "N" : "",
			f_DrawDistance ? "D" : "",
			f_DrawHealth ? "H" : ""
		);
	}

	ESP& ESP::GetInstance()
	{
		static ESP instance;
		return instance;
	}

	bool ESP::CheckPuzzleFinished(game::Entity* entity)
	{
#pragma region OldImplement
        //if (game::filters::puzzle::ElementalMonument.IsValid(entity))
        //{
        //    auto EntityGameObject = app::MoleMole_BaseEntity_get_rootGameObject(entity->raw(), nullptr);
        //    if (EntityGameObject == nullptr)
        //        return false;
        //    auto Transform = app::GameObject_GetComponentByName(EntityGameObject, string_to_il2cppi("Transform"), nullptr);
        //    auto child = app::Transform_GetChild(reinterpret_cast<app::Transform*>(Transform), 2, nullptr);
        //    auto pre_status = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(child), nullptr);
        //    auto status = app::GameObject_get_active(reinterpret_cast<app::GameObject*>(pre_status), nullptr);

        //    return status;//if monument finished - returns true
        //}
        //else if (game::filters::puzzle::ElectroSeelie.IsValid(entity))
        //{
        //    auto EntityGameObject = app::MoleMole_BaseEntity_get_rootGameObject(entity->raw(), nullptr);
        //    if (EntityGameObject == nullptr)
        //        return false;
        //    auto Transform = app::GameObject_GetComponentByName(EntityGameObject, string_to_il2cppi("Transform"), nullptr);
        //    auto child = app::Transform_GetChild(reinterpret_cast<app::Transform*>(Transform), 1, nullptr);
        //    auto pre_status = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(child), nullptr);
        //    auto status = app::GameObject_get_active(reinterpret_cast<app::GameObject*>(pre_status), nullptr);

        //    return status;//if seelie finished - returns true
        //}
        //else if (game::filters::puzzle::BloattyFloatty.IsValid(entity))
        //{
        //    auto EntityGameObject = app::MoleMole_BaseEntity_get_rootGameObject(entity->raw(), nullptr);
        //    if (EntityGameObject == nullptr)
        //        return false;
        //    auto Transform = app::GameObject_GetComponentByName(EntityGameObject, string_to_il2cppi("Transform"), nullptr);
        //    auto child = app::Transform_GetChild(reinterpret_cast<app::Transform*>(Transform), 0, nullptr);
        //    auto pre_status = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(child), nullptr);
        //    //now we take childs from 1st child
        //    auto Transform1 = app::GameObject_GetComponentByName(pre_status, string_to_il2cppi("Transform"), nullptr);
        //    auto child1 = app::Transform_GetChild(reinterpret_cast<app::Transform*>(Transform1), 1, nullptr);
        //    auto child2 = app::Transform_GetChild(reinterpret_cast<app::Transform*>(Transform1), 2, nullptr);
        //    auto child3 = app::Transform_GetChild(reinterpret_cast<app::Transform*>(Transform1), 3, nullptr);

        //    auto pre_status1 = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(child1), nullptr);
        //    auto pre_status2 = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(child2), nullptr);
        //    auto pre_status3 = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(child3), nullptr);

        //    auto status1 = app::GameObject_get_active(reinterpret_cast<app::GameObject*>(pre_status1), nullptr);
        //    auto status2 = app::GameObject_get_active(reinterpret_cast<app::GameObject*>(pre_status2), nullptr);
        //    auto status3 = app::GameObject_get_active(reinterpret_cast<app::GameObject*>(pre_status3), nullptr);

        //    if (status1 || status2 || status3)//if even one of them active - plant isn't finished
        //        return false;
        //    else
        //        return true;
        //}
        //else
        //    return false;
#pragma endregion
		auto& entityAppearManager = EntityAppearManager::GetInstance();
		auto& gadgetState = entityAppearManager.m_Gadgets[entity->runtimeID()].gadgetState;
		if ((game::filters::puzzle::ElementalMonument.IsValid(entity) && gadgetState == app::GadgetState__Enum::GearStop) // if ElementalMonument is locked
			|| (game::filters::puzzle::TheWithering.IsValid(entity) && gadgetState == app::GadgetState__Enum::GearStart)) // if DeathZoneCore is destroyable
			return false;
		if (entityAppearManager.m_Gadgets[entity->runtimeID()].gadgetId == 70540012)  //Currently, only GlazeLily has this problem. If you find more similar Gadgets, just add the ID at the end.
			return false;
		return gadgetState == app::GadgetState__Enum::GearStart || gadgetState == app::GadgetState__Enum::GearStop || gadgetState == app::GadgetState__Enum::GearAction1;
	}


    bool ESP::isBuriedChest(game::Entity* entity)
    {
        if (entity->name().find("_WorldArea_Operator") != std::string::npos)
        {
            auto entityGameObject = app::MoleMole_BaseEntity_get_rootGameObject(entity->raw(), nullptr);
			if (entityGameObject == nullptr)
				return false;
            auto transform = app::GameObject_GetComponentByName(entityGameObject, string_to_il2cppi("Transform"), nullptr);
            auto child = app::Transform_FindChild(reinterpret_cast<app::Transform*>(transform), string_to_il2cppi("CircleR2H2"), nullptr);
            if (child == nullptr)
                return false;

			auto configID = entity->raw()->fields._configID_k__BackingField;
			//LOG_DEBUG("%d", configID);
			if (configID != 70360001 && configID != 70360286)
				return false;

            return true;
        }
        return false;
    }

	void ESP::GetNpcName(std::string& name)
	{
		std::regex Avatars("Avatar_.*_.*_(\\D[a-z]+).*");
		std::regex NPCDefault(".*_.*_([A-Z][a-z]+).*");
		std::regex NPCAdvanced(".*_.*_.*_(([A-Z][a-z]+)([A-Z][a-z]+)).*");

		std::smatch m;
		
		if (name.find("Megamoth") != std::string::npos)
		{
			char AA[] = "Megamoth";
			name = AA;
			return;
		}
		if (name.find("Aranara") != std::string::npos)
		{
			char AA[] = "Aranara";
			name = AA;
			return;
		}
		else if (name.find("Kanban") != std::string::npos)
		{
			char AA[] = "Paimon";
			name = AA;
			return;
		}
		else if (std::regex_match(name, m, Avatars))
		{
			name = m[1].str();
			return;
		}
		else if (std::regex_match(name, m, NPCAdvanced))
		{
			name = m[2].str() + " " + m[3].str();
			return;
		}
		else if (std::regex_match(name, m, NPCDefault))
		{
			name = m[1].str();
			return;
		}
		else
		{
			name = "ERROR";
		}
		
	}

	void ESP::AddFilter(const std::string& section, const std::string& name, game::IEntityFilter* filter)
	{
		if (m_Sections.count(section) == 0)
			m_Sections[section] = {};

		auto& filters = m_Sections[section];
		esp::ESPItem newItem(name, ImColor(120, 120, 120, 255), {}, name);
		filters.push_back({ config::CreateField<esp::ESPItem>(name, fmt::format("ESP::Filters::{}", section), false, newItem), filter });
	}

	void ESP::DrawSection(const std::string& section, const Filters& filters)
	{
		std::vector<const FilterInfo*> validFilters;

		for (auto& info : filters)
		{
			const auto& filterName = info.first.value().m_Name;

			auto it = std::search(
				filterName.begin(), filterName.end(),
				m_Search.begin(), m_Search.end(),
				[](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
			);

			if (it != filterName.end())
				validFilters.push_back(&info);
		}

		if (validFilters.empty())
			return;

		bool checked = std::all_of(validFilters.begin(), validFilters.end(), [](const FilterInfo* filter) {  return filter->first.value().m_Enabled; });
		bool changed = false;

		if (ImGui::BeginSelectableGroupPanel(Translator::RuntimeTranslate(section.c_str()).c_str(), checked, changed, true))
		{
			for (auto& info : validFilters)
			{
				ImGui::PushID(info);
				DrawFilterField(info->first);
				ImGui::PopID();
			}

			ImGui::Spacing();

			if (ImGui::TreeNode(this, _TR("Hotkeys")))
			{
				for (auto& info : validFilters)
				{
					auto& field = info->first;
					ImGui::PushID(info);

					auto& hotkey = field.value().m_Hotkey;
					if (InputHotkey(Translator::RuntimeTranslate(field.name()).c_str(), &hotkey, true))
						field.FireChanged();

					ImGui::PopID();
				}

				ImGui::TreePop();
			}
		}
		ImGui::EndSelectableGroupPanel();

		if (changed)
		{
			for (auto& info : validFilters)
			{
				info->first.value().m_Enabled = checked;
				info->first.FireChanged();
			}
		}
	}

	void ESP::DrawCustomSection()
	{
		std::vector<const CustomFilterInfo*> validFilters;

		for (auto& info : m_CustomFilterInfos)
		{
			const auto& filterName = info.first.value().m_Name;

			auto it = std::search(
				filterName.begin(), filterName.end(),
				m_Search.begin(), m_Search.end(),
				[](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
			);

			if (it != filterName.end())
				validFilters.push_back(&info);
		}

		if (validFilters.empty())
			return;

		bool checked = std::all_of(validFilters.begin(), validFilters.end(), [](const CustomFilterInfo* filter) {  return filter->first.value().m_Enabled; });
		bool changed = false;

		if (ImGui::BeginSelectableGroupPanel(_TR("Custom filters"), checked, changed, true))
		{
			for (auto& info : validFilters)
			{
				ImGui::PushID(info);
				DrawFilterField(info->first);
				ImGui::PopID();
			}

			ImGui::Spacing();

			if (ImGui::TreeNode(this, _TR("Hotkeys")))
			{
				for (auto& info : validFilters)
				{
					auto& field = info->first;
					ImGui::PushID(info);

					auto& hotkey = field.value().m_Hotkey;
					if (InputHotkey(Translator::RuntimeTranslate(field.name()).c_str(), &hotkey, true))
						field.FireChanged();

					ImGui::PopID();
				}

				ImGui::TreePop();
			}
		}
		ImGui::EndSelectableGroupPanel();

		if (changed)
		{
			for (auto& info : validFilters)
			{
				info->first.value().m_Enabled = checked;
				info->first.FireChanged();
			}
		}
	}

	void FilterItemSelector(const char* label, ImTextureID image, const config::Field<esp::ESPItem>& field, const ImVec2& size = ImVec2(200, 0), float icon_size = 30);

	void ESP::DrawFilterField(const config::Field<esp::ESPItem>& field)
	{
		auto imageInfo = ImageLoader::GetImage(util::Unsplit(field.value().m_Name));
		FilterItemSelector(field.value().m_Name.c_str(), imageInfo ? imageInfo->textureID : nullptr, field);
	}

	void ESP::DrawCustomFilterNames()
	{
		const float clipSize = static_cast<float>(min(m_CustomFilterNames.size(), 15) + 1); // Number of rows in table as initial view. Past this is scrollbar territory.
		static ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;
		static ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAvailWidth;

		static int rowToDelete = -1;
		if (ImGui::BeginTable("Names", 2, flags, ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * clipSize), 0.0f))
		{
			ImGui::TableSetupColumn(_TR("Name"), ImGuiTableColumnFlags_WidthStretch, 0.0, 0);
			ImGui::TableSetupColumn(_TR("Actions"), ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(m_CustomFilterNames.size()));
			while (clipper.Step())
				for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
				{
					auto name = m_CustomFilterNames[row_n];
					ImGui::PushID((name + ":" + std::to_string(row_n)).c_str());
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					auto isSelected = false;
					auto isItemClicked = false;

					if (ImGui::Selectable(name.c_str(), &isSelected, selectableFlags))
						isItemClicked = isSelected && (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right));

					ImGui::TableNextColumn();

					if (ImGui::SmallButton(_TR("Delete")))
						rowToDelete = row_n;

					if (isItemClicked)
					{
						i_CustomFilterNameEditId = row_n;
						m_CustomFilterNameToAdd = m_CustomFilterNames.at(row_n);
					}

					ImGui::PopID();
				}

			ImGui::EndTable();
		}

		if (rowToDelete > -1)
		{
			m_CustomFilterNames.erase(m_CustomFilterNames.begin() + rowToDelete);
			i_CustomFilterNameEditId = -1;
			rowToDelete = -1;
		}
	}

	void ESP::DrawCustomFiltersTable()
	{
		const float clipSize = static_cast<float>(min(m_CustomFilters.size(), 15) + 1); // Number of rows in table as initial view. Past this is scrollbar territory.
		static ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;
		static ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAvailWidth;

		static int rowToDelete = -1;
		if (ImGui::BeginTable("Filters", 2, flags, ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * clipSize), 0.0f))
		{
			ImGui::TableSetupColumn(_TR("Display name"), ImGuiTableColumnFlags_WidthStretch, 0.0, 0);
			ImGui::TableSetupColumn(_TR("Actions"), ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(m_CustomFilters.size()));
			while (clipper.Step())
				for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
				{
					auto& filter = m_CustomFilters[row_n];
					ImGui::PushID((filter->m_FilterName + ":" + std::to_string(row_n)).c_str());
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					auto isSelected = false;
					auto isItemClicked = false;

					if (ImGui::Selectable(filter->m_FilterName.c_str(), &isSelected, selectableFlags))
						isItemClicked = isSelected && (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right));

					ImGui::TableNextColumn();

					if (ImGui::SmallButton(_TR("Delete")))
						rowToDelete = row_n;

					if (isItemClicked)
					{
						i_CustomFiltersEditId = row_n;
						i_CustomFilterNameEditId = -1;
						m_CustomFilterUiName = filter->m_FilterName;
						f_CustomFilterType = filter->m_Type;
						m_CustomFilterNameToAdd = "";
						m_CustomFilterNames = std::vector(filter->m_Names);
					}

					ImGui::PopID();
				}

			ImGui::EndTable();
		}

		if (rowToDelete > -1)
		{
			i_CustomFiltersEditId = -1;
			i_CustomFilterNameEditId = -1;
			m_CustomFilters.erase(m_CustomFilters.begin() + rowToDelete);
			m_CustomFilterInfos.erase(m_CustomFilterInfos.begin() + rowToDelete);
			rowToDelete = -1;
			SaveCustomFilters();
		}
	}

	void ESP::DrawCustomFiltersUi()
	{
		if (!f_ShowCustomFiltersWindow.value())
			return;

		static ImGuiWindowFlags flags = ImGuiWindowFlags_None /*ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize*/;

		auto warningText = _TR("NOTE: Custom filter is only for advanced users or contributors! You must know what are you doing.");
		auto warningTextSz = ImGui::CalcTextSize(warningText);

		ImGui::SetNextWindowSizeConstraints(ImVec2(warningTextSz.x + 30.f, 400.f), ImVec2(INFINITY, INFINITY));

		if (ImGui::Begin(_TR("Custom filters"), &f_ShowCustomFiltersWindow.value(), flags))
		{
			ImGui::Text(_TR("NOTE: Custom filter is only for advanced users or contributors! You must know what are you doing."));

			ImGui::Spacing();

			if (ImGui::BeginGroupPanel(_TR("New filter")))
			{
				ImGui::InputText(_TR("Display name"), &m_CustomFilterUiName);
				ConfigWidget(_TR("Type"), f_CustomFilterType);

				ImGui::Spacing();
				ImGui::InputText(_TR("Name"), &m_CustomFilterNameToAdd);

				ImGui::SameLine();
				auto isNamesInEditMode = i_CustomFilterNameEditId > -1;
				if (ImGui::Button(isNamesInEditMode ? _TR("Update name") : _TR("Add")))
				{
					if (isNamesInEditMode)
						m_CustomFilterNames[i_CustomFilterNameEditId] = m_CustomFilterNameToAdd;
					else
						m_CustomFilterNames.push_back(m_CustomFilterNameToAdd);

					i_CustomFilterNameEditId = -1;
					m_CustomFilterNameToAdd = "";
				}

				if (isNamesInEditMode)
				{
					ImGui::SameLine();
					if (ImGui::Button(_TR("Cancel editing name")))
					{
						i_CustomFilterNameEditId = -1;
						m_CustomFilterNameToAdd = "";
					}
				}

				DrawCustomFilterNames();

				auto isEditMode = i_CustomFiltersEditId > -1;
				if (isEditMode)
				{
					if (ImGui::Button(_TR("Cancel editing")))
					{
						i_CustomFiltersEditId = -1;
						i_CustomFilterNameEditId = -1;

						m_CustomFilterUiName = "";
						f_CustomFilterType = app::EntityType__Enum_1::None;
						m_CustomFilterNameToAdd = "";

						m_CustomFilterNames = {};
					}

					ImGui::SameLine();
				}

				auto addOrUpdateButtonName = isEditMode ? _TR("Update filter") : _TR("Add filter");
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcButtonSize(addOrUpdateButtonName).x);
				if (ImGui::Button(addOrUpdateButtonName))
				{
					auto filter = ESPCustomFilter(m_CustomFilterUiName, f_CustomFilterType.value(), m_CustomFilterNames);
					if (isEditMode)
					{
						auto existingFilter = m_CustomFilters.at(i_CustomFiltersEditId);
						existingFilter->m_FilterName = m_CustomFilterUiName;
						existingFilter->SetType(f_CustomFilterType.value());
						existingFilter->SetNames(m_CustomFilterNames);

						SaveCustomFilters();
					}
					else
					{
						AddCustomFilter(filter, true);
					}

					i_CustomFiltersEditId = -1;
					i_CustomFilterNameEditId = -1;
					m_CustomFilterUiName = "";
					m_CustomFilterNames = {};
					f_CustomFilterType = app::EntityType__Enum_1::None;
				}
			}
			ImGui::EndGroupPanel();

			ImGui::Spacing();

			DrawCustomFiltersTable();
		}
		ImGui::End();
	}

	void ESP::DrawExternal()
	{
		esp::render::PrepareFrame();

		if (CheatManagerBase::IsMenuShowed())
			DrawCustomFiltersUi();

		auto& esp = ESP::GetInstance();
		if (!esp.f_Enabled->enabled())
			return;

		auto& entityManager = game::EntityManager::instance();

        for (const auto entity : entityManager.entities())
        {
            if (entityManager.avatar()->distance(entity) > esp.f_Range)
                continue;

            for (const auto &filters : m_Sections | std::views::values)
            {
                for (const auto &[field, filter] : filters)
                {
                    const auto &entry = field.value();
                    const uint64_t name_hash = xxHash64(entry.m_Name);
                    if (!entry.m_Enabled || !m_FilterExecutor.ApplyFilter(entity, filter))
                        continue;

                    switch (name_hash)
                    {
                    //case "Elemental Monument"_h:
                    //case "Bloatty Floatty"_h:
                    //case "Electro Seelie"_h:
                    //    if (f_HideCompleted && ESP::CheckPuzzleFinished(entity))
                    //        break;
                    //    esp::render::DrawEntity(entry.m_Name, Translator::RuntimeTranslate(entry.m_Name), entity, entry.m_Color, entry.m_ContrastColor);
                    //    break;

                    case "Buried Chest"_h:
                        if (isBuriedChest(entity))
                        {
                            esp::render::DrawEntity(entry.m_Name, Translator::RuntimeTranslate(entry.m_Name), entity, entry.m_Color, entry.m_ContrastColor);
                        }
                        break;

                    case "Npc"_h:
                    case "AvatarOwn"_h:
                    case "AvatarTeammate"_h:
                        if (isBuriedChest(entity))
                            continue;
                        if (entity->type() == app::EntityType__Enum_1::Avatar ||
                            entity->type() == app::EntityType__Enum_1::NPC)
                        {
                            std::string name = entity->name();
                            GetNpcName(name);
                            esp::render::DrawEntity(entry.m_Name, Translator::RuntimeTranslate(name), entity, entry.m_Color, entry.m_ContrastColor);
                        }
                        break;

                    default:
                        if (f_HideCompleted && ESP::CheckPuzzleFinished(entity))
                            break;
                        esp::render::DrawEntity(entry.m_Name, Translator::RuntimeTranslate(entry.m_Name), entity, entry.m_Color, entry.m_ContrastColor);
                        break;
                    }
                    break;
                }
            }

            for (const auto &[field, filter] : m_CustomFilterInfos)
            {
                const auto &entry = field.value();
                if (!entry.m_Enabled || !m_FilterExecutor.ApplyFilter(entity, filter))
                    continue;

                esp::render::DrawEntity(entry.m_Name, Translator::RuntimeTranslate(entry.m_Name), entity, entry.m_Color, entry.m_ContrastColor);
            }
        }
	}

	void ESP::AddCustomFilter(ESPCustomFilter filter, bool convertToJson)
	{
		auto ptrFilter = std::make_shared<ESPCustomFilter>(filter);
		m_CustomFilters.push_back(ptrFilter);

		esp::ESPItem newItem(filter.m_FilterName, ImColor(120, 120, 120, 255), {}, "CustomFilterItem");
		m_CustomFilterInfos.push_back(CustomFilterInfo(config::CreateField<esp::ESPItem>(filter.m_FilterName, "ESP::Filters::Custom", false, newItem), ptrFilter.get()));

		if (convertToJson)
			SaveCustomFilters();
	}

	void ESP::OnKeyUp(short key, bool& cancelled)
	{
		for (auto& [section, filters] : m_Sections)
		{
			for (auto& [field, filter] : filters)
			{
				auto& entry = field.value();
				if (entry.m_Hotkey.IsPressed(key))
				{
					entry.m_Enabled = !entry.m_Enabled;
					field.FireChanged();
				}
			}
		}
	}

	void ESP::SaveCustomFilters()
	{
		std::vector<nlohmann::json> jItems = {};
		for (auto& filter : m_CustomFilters)
		{
			jItems.push_back(config::converters::ToJson(*filter.get()));
		}

		f_CustomFilterJson = config::converters::ToJson(jItems);
	}

	void FilterItemSelector(const char* label, ImTextureID image, const config::Field<esp::ESPItem>& field, const ImVec2& size, float icon_size)
	{

		// Init ImGui
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		ImGuiIO& io = g.IO;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		
		auto translatedLabel = Translator::RuntimeTranslate(label);
		const ImVec2 label_size = ImGui::CalcTextSize(translatedLabel.c_str(), NULL, true);
		const ImVec2 item_size = ImGui::CalcItemSize(size, ImGui::CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f + 20.0f);

		float region_max_x = ImGui::GetContentRegionMaxAbs().x;
		if (region_max_x - window->DC.CursorPos.x < item_size.x)
			ImGui::Spacing();

		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + item_size);
		const ImRect total_bb(window->DC.CursorPos, { frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Max.y });

		ImGui::ItemSize(total_bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(total_bb, id))
		{
			ImGui::SameLine();
			return;
		}
		const bool hovered = ImGui::ItemHoverable(frame_bb, id);
		if (hovered)
		{
			ImGui::SetHoveredID(id);
			g.MouseCursor = ImGuiMouseCursor_Hand;
		}

		const bool lmb_click = hovered && io.MouseClicked[0];
		if (lmb_click)
		{
			field.value().m_Enabled = !field.value().m_Enabled;
			field.FireChanged();
			ImGui::FocusWindow(window);
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
		}

		const bool rmb_click = hovered && io.MouseClicked[ImGuiMouseButton_Right];

		ImGuiWindow* picker_active_window = NULL;

		static bool color_changed = false;
		static ImGuiID opened_id = 0;
		if (rmb_click)
		{
			// Store current color and open a picker
			g.ColorPickerRef = ImVec4(field.value().m_Color);
			ImGui::OpenPopup("picker");
			ImGui::SetNextWindowPos(g.LastItemData.Rect.GetBL() + ImVec2(0.0f, style.ItemSpacing.y));
			opened_id = id;
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
		}

		if (ImGui::BeginPopup("picker"))
		{
			picker_active_window = g.CurrentWindow;
			//ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
			ImGuiColorEditFlags picker_flags = ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
			ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 12.0f); // Use 256 + bar sizes?
			color_changed |= ImGui::ColorPicker4("##picker", reinterpret_cast<float*>(&field.value().m_Color), picker_flags, &g.ColorPickerRef.x);
			ImGui::EndPopup();
		}

		bool popup_closed = id == opened_id && picker_active_window == NULL;
		if (popup_closed)
		{
			opened_id = 0;
			if (color_changed)
			{
				field.value().m_ContrastColor = ImGui::CalcContrastColor(field.value().m_Color);
				field.FireChanged();
				color_changed = false;
			}
		}

		const ImU32 border_color = ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + item_size.x, frame_bb.Min.y + item_size.y); // Not using frame_bb.Max because we have adjusted size

		float border_size = 2.0f;
		float border_rounding = 10.0f;

		ImVec2 render_pos = frame_bb.Min + style.FramePadding;
		ImGui::RenderFrame(clip_rect.Min, clip_rect.Max, field.value().m_Color, false, border_rounding);

		if (field.value().m_Enabled)
		{
			float check_mark_size = 13.0f;
			ImVec2 checkStart = { clip_rect.Max.x - check_mark_size - border_size, clip_rect.Min.y };
			ImGui::RenderFrame(checkStart - ImVec2(2.0f, 0), checkStart + ImVec2(check_mark_size + border_size, check_mark_size + border_size + 2.0f),
				ImGui::GetColorU32(ImGuiCol_FrameBg), false, border_rounding);
			ImGui::RenderCheckMark(window->DrawList, checkStart + ImVec2(0, 1.0f), 0xFFFFFFFF, check_mark_size - 1.0f);
		}

		window->DrawList->AddRect(clip_rect.Min, clip_rect.Max, border_color, border_rounding - 1.0f, 0, border_size);

		float y_center = frame_bb.Min.y + (frame_bb.Max.y - frame_bb.Min.y) / 2;

		ImVec2 image_start(frame_bb.Min.x + style.FramePadding.x + 5.0f, y_center - icon_size / 2);
		ImVec2 image_end(image_start.x + icon_size, image_start.y + icon_size);
		if (image != nullptr)
			window->DrawList->AddImageRounded(image, image_start, image_start + ImVec2(icon_size, icon_size), { 0.0f, 0.0f }, { 1.0f, 1.0f },
				ImColor(1.0f, 1.0f, 1.0f), 0.3f);

		bool pushed = ImGui::PushStyleColorWithContrast(field.value().m_Color, ImGuiCol_Text, ImColor(0, 0, 0), 2.0f);

		ImVec2 text_end(frame_bb.Max.x - style.FramePadding.x - border_size, y_center + label_size.y / 2);
		ImVec2 text_start(ImMax(image_end.x + style.FramePadding.x, text_end.x - label_size.x), y_center - label_size.y / 2);
		ImGui::RenderTextClipped(text_start, text_end, translatedLabel.c_str(), NULL, NULL, { 0, 0 }, &clip_rect);

		if (pushed)
			ImGui::PopStyleColor();

		ImGui::SameLine();

		return;
	}

#define ADD_FILTER_FIELD(section, name) AddFilter(util::MakeCapital(#section), util::SplitWords(#name), &game::filters::##section##::##name##)
	void ESP::InstallFilters()
	{
		ADD_FILTER_FIELD(collection, Book);
		ADD_FILTER_FIELD(collection, Viewpoint);
		ADD_FILTER_FIELD(collection, RadiantSpincrystal);
		ADD_FILTER_FIELD(collection, BookPage);
		ADD_FILTER_FIELD(collection, QuestInteract);
		ADD_FILTER_FIELD(collection, WoodenCrate);
		ADD_FILTER_FIELD(collection, GeoSigil);

		// Regular Chests
		ADD_FILTER_FIELD(chest, CommonChest);
		ADD_FILTER_FIELD(chest, ExquisiteChest);
		ADD_FILTER_FIELD(chest, PreciousChest);
		ADD_FILTER_FIELD(chest, LuxuriousChest);
		ADD_FILTER_FIELD(chest, RemarkableChest);
		// Other Chests
		//ADD_FILTER_FIELD(chest, BuriedChest);
		ADD_FILTER_FIELD(chest, SearchPoint);
		ADD_FILTER_FIELD(chest, CrudeChest);
		

		ADD_FILTER_FIELD(featured, Anemoculus);
		ADD_FILTER_FIELD(featured, CrimsonAgate);
		ADD_FILTER_FIELD(featured, Dendroculus);
		ADD_FILTER_FIELD(featured, Electroculus);
		ADD_FILTER_FIELD(featured, Electrogranum);
		ADD_FILTER_FIELD(featured, FishingPoint);
		ADD_FILTER_FIELD(featured, Geoculus);
		ADD_FILTER_FIELD(featured, ItemDrops);
		ADD_FILTER_FIELD(featured, KeySigil);
		ADD_FILTER_FIELD(featured, Lumenspar);
		ADD_FILTER_FIELD(featured, SacredSeal);
		ADD_FILTER_FIELD(featured, ShrineOfDepth);
		ADD_FILTER_FIELD(featured, TimeTrialChallenge);

		ADD_FILTER_FIELD(guide, BouncyMushroom);
		ADD_FILTER_FIELD(guide, CampfireTorch);
		ADD_FILTER_FIELD(guide, ClusterleafOfCultivation);
		ADD_FILTER_FIELD(guide, DayNightSwitchingMechanism);
		ADD_FILTER_FIELD(guide, DendroGranum);
		ADD_FILTER_FIELD(guide, DendroPile);
		ADD_FILTER_FIELD(guide, DendroRock);
		ADD_FILTER_FIELD(guide, EnkanomiyaPhaseGate);
		ADD_FILTER_FIELD(guide, MysteriousCarvings);
		ADD_FILTER_FIELD(guide, PhaseGate);
		ADD_FILTER_FIELD(guide, PlacesofEssenceWorship);
		ADD_FILTER_FIELD(guide, Pot);
		ADD_FILTER_FIELD(guide, RuinBrazier);
		ADD_FILTER_FIELD(guide, SandPile);
		ADD_FILTER_FIELD(guide, Stormstone);
		ADD_FILTER_FIELD(guide, TriangularMechanism);

		ADD_FILTER_FIELD(living, AvatarTeammate);
		ADD_FILTER_FIELD(living, AvatarOwn);
		ADD_FILTER_FIELD(living, BakeDanuki);
		ADD_FILTER_FIELD(living, BirdEgg);
		ADD_FILTER_FIELD(living, Boar);
		ADD_FILTER_FIELD(living, ButterflyWings);
		ADD_FILTER_FIELD(living, Cat);
		ADD_FILTER_FIELD(living, Crab);
		ADD_FILTER_FIELD(living, Crane);
		ADD_FILTER_FIELD(living, Crow);
		ADD_FILTER_FIELD(living, CrystalCore);
		ADD_FILTER_FIELD(living, Dog);
		ADD_FILTER_FIELD(living, DuskBird);
		ADD_FILTER_FIELD(living, Eel);
		ADD_FILTER_FIELD(living, Falcon);
		ADD_FILTER_FIELD(living, Finch);
		ADD_FILTER_FIELD(living, Fish);
		ADD_FILTER_FIELD(living, Fox);
		ADD_FILTER_FIELD(living, Frog);
		ADD_FILTER_FIELD(living, Inu);
		ADD_FILTER_FIELD(living, Kitsune);
		ADD_FILTER_FIELD(living, LizardTail);
		ADD_FILTER_FIELD(living, LucklightFly);
		ADD_FILTER_FIELD(living, LuminescentSpine);
		ADD_FILTER_FIELD(living, Npc);
		ADD_FILTER_FIELD(living, Onikabuto);
		ADD_FILTER_FIELD(living, Pigeon);
		ADD_FILTER_FIELD(living, Salamander);
		ADD_FILTER_FIELD(living, Scarab);
		ADD_FILTER_FIELD(living, Squirrel);
		ADD_FILTER_FIELD(living, Starconch);
		ADD_FILTER_FIELD(living, Weasel);
		ADD_FILTER_FIELD(living, Wigeon);

		ADD_FILTER_FIELD(mineral, AmethystLump);
		ADD_FILTER_FIELD(mineral, ArchaicStone);
		ADD_FILTER_FIELD(mineral, CorLapis);
		ADD_FILTER_FIELD(mineral, CrystalChunk);
		ADD_FILTER_FIELD(mineral, CrystalMarrow);
		ADD_FILTER_FIELD(mineral, DunlinsTooth);
		ADD_FILTER_FIELD(mineral, ElectroCrystal);
		ADD_FILTER_FIELD(mineral, IronChunk);
		ADD_FILTER_FIELD(mineral, NoctilucousJade);
		ADD_FILTER_FIELD(mineral, MagicalCrystalChunk);
		ADD_FILTER_FIELD(mineral, ScarletQuartz);
		ADD_FILTER_FIELD(mineral, Starsilver);
		ADD_FILTER_FIELD(mineral, WhiteIronChunk);

		// Trounce. Arranged by appearance in-game.
		ADD_FILTER_FIELD(monster, Andrius);
		ADD_FILTER_FIELD(monster, Azhdaha);
		ADD_FILTER_FIELD(monster, Dvalin);
		ADD_FILTER_FIELD(monster, Signora);
		ADD_FILTER_FIELD(monster, Shogun);
		ADD_FILTER_FIELD(monster, ShoukiNoKami);
		ADD_FILTER_FIELD(monster, Tartaglia);
		// Bosses. Arranged by "type" then alphabetical.
		// Regisvines.
		ADD_FILTER_FIELD(monster, CryoRegisvine);
		ADD_FILTER_FIELD(monster, ElectroRegisvine);
		ADD_FILTER_FIELD(monster, PyroRegisvine);
		// Hypostases.
		ADD_FILTER_FIELD(monster, AnemoHypostasis);
		ADD_FILTER_FIELD(monster, CryoHypostasis);
		ADD_FILTER_FIELD(monster, ElectroHypostasis);
		ADD_FILTER_FIELD(monster, DendroHypostasis);
		ADD_FILTER_FIELD(monster, GeoHypostasis);
		ADD_FILTER_FIELD(monster, HydroHypostasis);
		ADD_FILTER_FIELD(monster, PyroHypostasis);
		// Vishaps.
		ADD_FILTER_FIELD(monster, CryoBathysmalVishap);
		ADD_FILTER_FIELD(monster, ElectroBathysmalVishap);
		ADD_FILTER_FIELD(monster, PrimoGeovishap);
		// Oceanids.
		ADD_FILTER_FIELD(monster, Oceanid);
		ADD_FILTER_FIELD(monster, ThunderManifestation);
		// Khaenri'ah.
		ADD_FILTER_FIELD(monster, PerpetualMechanicalArray);
		ADD_FILTER_FIELD(monster, RuinSerpent);
		// Others.
		ADD_FILTER_FIELD(monster, GoldenWolflord);
		ADD_FILTER_FIELD(monster, MaguuKenki);
		// Sumeru
		ADD_FILTER_FIELD(monster, AeonblightDrake);
		ADD_FILTER_FIELD(monster, AlgorithmOfSemiIntransientMatrixOfOverseerNetwork);
		ADD_FILTER_FIELD(monster, JadeplumeTerrorshroom);
		ADD_FILTER_FIELD(monster, SetekhWenut);
		// Regular. Alphabetical.
		ADD_FILTER_FIELD(monster, AbyssMage);
		ADD_FILTER_FIELD(monster, BlackSerpentKnight);
		ADD_FILTER_FIELD(monster, BlessboneFlyingSerpent);
		ADD_FILTER_FIELD(monster, BlessboneRedVulture);
		ADD_FILTER_FIELD(monster, BlessboneScorpion);
		ADD_FILTER_FIELD(monster, Cicin);
		ADD_FILTER_FIELD(monster, ElectroAbyssLector);
		ADD_FILTER_FIELD(monster, Eremite);
		ADD_FILTER_FIELD(monster, EyeOfTheStorm);
		ADD_FILTER_FIELD(monster, FatuiAgent);
		ADD_FILTER_FIELD(monster, FatuiCicinMage);
		ADD_FILTER_FIELD(monster, FatuiMirrorMaiden);
		ADD_FILTER_FIELD(monster, FatuiSkirmisher);
		ADD_FILTER_FIELD(monster, FloatingFungus);
		ADD_FILTER_FIELD(monster, FlyingSerpent);
		ADD_FILTER_FIELD(monster, Geovishap);
		ADD_FILTER_FIELD(monster, GeovishapHatchling);
		ADD_FILTER_FIELD(monster, GroundedShroom);
		ADD_FILTER_FIELD(monster, Hilichurl);
		ADD_FILTER_FIELD(monster, HydroAbyssHerald);
		ADD_FILTER_FIELD(monster, HydroBathysmalVishap);
		ADD_FILTER_FIELD(monster, Kairagi);
		ADD_FILTER_FIELD(monster, Millelith);
		ADD_FILTER_FIELD(monster, Mitachurl);
		ADD_FILTER_FIELD(monster, Nobushi);
		ADD_FILTER_FIELD(monster, OceanidBoar);
		ADD_FILTER_FIELD(monster, OceanidCrab);
		ADD_FILTER_FIELD(monster, OceanidCrane);
		ADD_FILTER_FIELD(monster, OceanidFalcon);
		ADD_FILTER_FIELD(monster, OceanidFinch);
		ADD_FILTER_FIELD(monster, OceanidFrog);
		ADD_FILTER_FIELD(monster, OceanidSquirrel);
		ADD_FILTER_FIELD(monster, OceanidWigeon);
		ADD_FILTER_FIELD(monster, PrimalConstruct);
		ADD_FILTER_FIELD(monster, PyroAbyssLector);
		ADD_FILTER_FIELD(monster, Rifthound);
		ADD_FILTER_FIELD(monster, RifthoundWhelp);
		ADD_FILTER_FIELD(monster, RishbolandTiger);
		ADD_FILTER_FIELD(monster, RuinDrake);
		ADD_FILTER_FIELD(monster, RuinGrader);
		ADD_FILTER_FIELD(monster, RuinGuard);
		ADD_FILTER_FIELD(monster, RuinHunter);
		ADD_FILTER_FIELD(monster, RuinSentinel);
		ADD_FILTER_FIELD(monster, Samachurl);
		ADD_FILTER_FIELD(monster, SangonomiyaCohort);
		ADD_FILTER_FIELD(monster, Scorpion);
		ADD_FILTER_FIELD(monster, ShadowyHusk);
		ADD_FILTER_FIELD(monster, ShaggySumpterBeast);
		ADD_FILTER_FIELD(monster, ShogunateInfantry);
		ADD_FILTER_FIELD(monster, Slime);
		ADD_FILTER_FIELD(monster, Specter);
		ADD_FILTER_FIELD(monster, Spincrocodile);
		ADD_FILTER_FIELD(monster, StretchyFungus);
		ADD_FILTER_FIELD(monster, TreasureHoarder);
		ADD_FILTER_FIELD(monster, UnusualHilichurl);
		ADD_FILTER_FIELD(monster, Vulture);
		ADD_FILTER_FIELD(monster, WhirlingFungus);
		ADD_FILTER_FIELD(monster, Whopperflower);
		ADD_FILTER_FIELD(monster, WingedShroom);

		ADD_FILTER_FIELD(plant, AjilenakhNut);
		ADD_FILTER_FIELD(plant, AmakumoFruit);
		ADD_FILTER_FIELD(plant, Apple);
		ADD_FILTER_FIELD(plant, BambooShoot);
		ADD_FILTER_FIELD(plant, Berry);
		ADD_FILTER_FIELD(plant, CallaLily);
		ADD_FILTER_FIELD(plant, Carrot);
		ADD_FILTER_FIELD(plant, Cecilia);
		ADD_FILTER_FIELD(plant, DandelionSeed);
		ADD_FILTER_FIELD(plant, Dendrobium);
		ADD_FILTER_FIELD(plant, FlamingFlowerStamen);
		ADD_FILTER_FIELD(plant, FluorescentFungus);
		ADD_FILTER_FIELD(plant, GlazeLily);
		ADD_FILTER_FIELD(plant, HarraFruit);
		ADD_FILTER_FIELD(plant, HennaBerry);
		ADD_FILTER_FIELD(plant, Horsetail);
		ADD_FILTER_FIELD(plant, JueyunChili);
		ADD_FILTER_FIELD(plant, KalpalataLotus);
		ADD_FILTER_FIELD(plant, LavenderMelon);
		ADD_FILTER_FIELD(plant, LotusHead);
		ADD_FILTER_FIELD(plant, Matsutake);
		ADD_FILTER_FIELD(plant, Mint);
		ADD_FILTER_FIELD(plant, MistFlowerCorolla);
		ADD_FILTER_FIELD(plant, Mushroom);
		ADD_FILTER_FIELD(plant, NakuWeed);
		ADD_FILTER_FIELD(plant, NilotpalaLotus);
		ADD_FILTER_FIELD(plant, Padisarah);
		ADD_FILTER_FIELD(plant, PhilanemoMushroom);
		ADD_FILTER_FIELD(plant, Pinecone);
		ADD_FILTER_FIELD(plant, Qingxin);
		ADD_FILTER_FIELD(plant, Radish);
		ADD_FILTER_FIELD(plant, RukkhashavaMushroom);
		ADD_FILTER_FIELD(plant, SakuraBloom);
		ADD_FILTER_FIELD(plant, SangoPearl);
		ADD_FILTER_FIELD(plant, SeaGanoderma);
		ADD_FILTER_FIELD(plant, Seagrass);
		ADD_FILTER_FIELD(plant, SilkFlower);
		ADD_FILTER_FIELD(plant, SmallLampGrass);
		ADD_FILTER_FIELD(plant, Snapdragon);
		ADD_FILTER_FIELD(plant, SumeruRose);
		ADD_FILTER_FIELD(plant, Sunsettia);
		ADD_FILTER_FIELD(plant, SweetFlower);
		ADD_FILTER_FIELD(plant, Valberry);
		ADD_FILTER_FIELD(plant, Violetgrass);
		ADD_FILTER_FIELD(plant, Viparyas);
		ADD_FILTER_FIELD(plant, WindwheelAster);
		ADD_FILTER_FIELD(plant, Wolfhook);
		ADD_FILTER_FIELD(plant, ZaytunPeach);

		ADD_FILTER_FIELD(puzzle, AncientRime);
		ADD_FILTER_FIELD(puzzle, BakeDanuki);
		ADD_FILTER_FIELD(puzzle, BloattyFloatty);
		ADD_FILTER_FIELD(puzzle, CubeDevices);
		ADD_FILTER_FIELD(puzzle, EightStoneTablets);
		ADD_FILTER_FIELD(puzzle, ElectricConduction);
		ADD_FILTER_FIELD(puzzle, ElectroSeelie);
		ADD_FILTER_FIELD(puzzle, ElementalMonument);
		ADD_FILTER_FIELD(puzzle, EverlightCell);
		ADD_FILTER_FIELD(puzzle, FloatingAnemoSlime);
		ADD_FILTER_FIELD(puzzle, Geogranum);
		ADD_FILTER_FIELD(puzzle, GeoPuzzle);
		ADD_FILTER_FIELD(puzzle, IllusionMural);
		ADD_FILTER_FIELD(puzzle, LargeRockPile);
		ADD_FILTER_FIELD(puzzle, LightningStrikeProbe);
		ADD_FILTER_FIELD(puzzle, LightUpTilePuzzle);
		ADD_FILTER_FIELD(puzzle, LumenCage);
		ADD_FILTER_FIELD(puzzle, LuminousSeelie);
		ADD_FILTER_FIELD(puzzle, MistBubble);
		ADD_FILTER_FIELD(puzzle, NurseriesInTheWilds);
		ADD_FILTER_FIELD(puzzle, OozingConcretions);
		ADD_FILTER_FIELD(puzzle, PhantasmalGate);
		ADD_FILTER_FIELD(puzzle, PirateHelm);
		ADD_FILTER_FIELD(puzzle, PressurePlate);
		ADD_FILTER_FIELD(puzzle, PrimalEmber);
		ADD_FILTER_FIELD(puzzle, PrimalObelisk); // might be shared entity
		ADD_FILTER_FIELD(puzzle, PrimalSandglass);
		ADD_FILTER_FIELD(puzzle, RelayStone);
		ADD_FILTER_FIELD(puzzle, SaghiraMachine);
		ADD_FILTER_FIELD(puzzle, SealLocations);
		ADD_FILTER_FIELD(puzzle, Seelie);
		ADD_FILTER_FIELD(puzzle, SeelieLamp);
		ADD_FILTER_FIELD(puzzle, SmallRockPile);
		ADD_FILTER_FIELD(puzzle, StonePillarSeal);
		ADD_FILTER_FIELD(puzzle, StormBarrier);
		ADD_FILTER_FIELD(puzzle, SumeruPuzzles);
		ADD_FILTER_FIELD(puzzle, SwordHilt);
		ADD_FILTER_FIELD(puzzle, Temari);
		ADD_FILTER_FIELD(puzzle, TheWithering);
		ADD_FILTER_FIELD(puzzle, TimeTrialChallengeCollection);
		ADD_FILTER_FIELD(puzzle, TorchPuzzle);
		ADD_FILTER_FIELD(puzzle, TriYanaSeeds);
		ADD_FILTER_FIELD(puzzle, UniqueRocks);
		ADD_FILTER_FIELD(puzzle, WarmingSeelie);
		ADD_FILTER_FIELD(puzzle, WindmillMechanism);
	}
#undef ADD_FILTER_FIELD

	void ESP::InstallCustomFilters()
	{
		if (f_CustomFilterJson.value().is_null() || !f_CustomFilterJson.value().is_array()) {
			LOG_WARNING("Failed to load custom filter data.");
			return;
		}

		auto jItems = f_CustomFilterJson.value().get<std::vector<nlohmann::json>>();
		for (auto& jObject : jItems)
		{
			try
			{
				auto filter = ESPCustomFilter();
				config::converters::FromJson(filter, jObject);

				AddCustomFilter(filter);
			}
			catch (const std::exception& e)
			{
				LOG_WARNING("Failed to initialize custom filter item.\n%s\n\nJSON dump:\n%s", e.what(), jObject.dump(4).c_str());
				continue;
			}
		}
	}
}
