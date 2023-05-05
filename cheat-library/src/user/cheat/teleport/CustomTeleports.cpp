#include "pch-il2cpp.h"
#include "CustomTeleports.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include "MapTeleport.h"
#include <cheat/game/util.h>
#include <misc/cpp/imgui_stdlib.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <imgui_internal.h>
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

namespace cheat::feature
{
	CustomTeleports::CustomTeleports() : Feature(),
		NFP(f_Enabled, "CustomTeleports", "Custom Teleports", false),
		NFP(f_Interpolate, "CustomTeleports", "Interpolate", false),
		NFP(f_Auto, "CustomTeleports", "Auto", false),
		NF(f_Speed, "CustomTeleports", 10.0f),
		NF(f_Next, "CustomTeleports", Hotkey(VK_OEM_6)),
		NF(f_Previous, "CustomTeleports", Hotkey(VK_OEM_4)),
		NF(f_DelayTime, "CustomTeleports", 20),
		dir(util::GetCurrentPath() /= "teleports"),
		nextTime(0)
	{
		f_Next.value().PressedEvent += MY_METHOD_HANDLER(CustomTeleports::OnNext);
		f_Previous.value().PressedEvent += MY_METHOD_HANDLER(CustomTeleports::OnPrevious);
		events::GameUpdateEvent += MY_METHOD_HANDLER(CustomTeleports::OnGameUpdate);
	}

	const FeatureGUIInfo& CustomTeleports::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Custom Teleports", "Teleport");
		return info;
	}

	void CustomTeleports::CheckFolder()
	{
		if (!std::filesystem::exists(dir))
			std::filesystem::create_directory(dir);
		else return;
	}

	bool CustomTeleports::ValidateTeleport(std::string name)
	{
		for (auto &Teleport : Teleports)
			if (Teleport.name == name)
				return false;
		if (name.find_first_of("\\/:*?\"<>|") != std::string::npos)
				return false;
		return true;
	}

	Teleport CustomTeleports::Teleport_(std::string name, app::Vector3 position, std::string description)
	{
		Teleport t(name, position, description);
		return t;
	}

	void CustomTeleports::SerializeTeleport(Teleport t)
	{
		Teleports.push_back(t);
		LOG_INFO("Teleport '%s' Loaded", t.name.c_str());
		CheckFolder();
		std::ofstream ofs(dir / (t.name + ".json"));
		nlohmann::json j;
		try
		{
			j["name"] = t.name;
			j["position"] = {t.position.x, t.position.y, t.position.z};
			j["description"] = t.description;
			ofs << j;
			ofs.close();
			LOG_INFO("Teleport '%s' Serialized.", t.name.c_str());
		} catch (std::exception e)
		{
			ofs.close();
			LOG_ERROR("Failed to serialize teleport: %s: %s", t.name.c_str(), e.what());
		}
	}

	 std::optional<Teleport> CustomTeleports::SerializeFromJson(std::string json, bool fromfile)
	{
		nlohmann::json j;
		try { j = nlohmann::json::parse(json); }
		catch (nlohmann::json::parse_error& e)
		{
			LOG_ERROR("Invalid JSON Format");
			LOG_ERROR("Failed to parse JSON: %s", e.what());
			return std::nullopt;
		}

		std::string teleportName;

		teleportName = j["name"];
		if (j["name"].is_null() && fromfile)
		{
			LOG_ERROR("No name found! Using File Name");
			teleportName = std::filesystem::path(json).stem().filename().string();
		}
		std::string description;
		if (j["description"].is_null()) description = "";
		else description = j["description"];
		return Teleport_(teleportName, { j["position"][0], j["position"][1], j["position"][2] }, description);

	}
	
	void CustomTeleports::ReloadTeleports()
	{
		auto result = std::filesystem::directory_iterator(dir);
		Teleports.clear();

		for (auto &file : result)
		{
			if (file.path().extension() == ".json")
			{
				std::ifstream ifs(file.path());
				std::string json;
				std::getline(ifs, json);
				auto t = SerializeFromJson(json, true);
				if(t.has_value()) SerializeTeleport(t.value());
			}
		}
	}

	float PositionDistance(app::Vector3 a, app::Vector3 b)
	{
		const double distance = sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
		return static_cast<float>(distance);
	}

	void CustomTeleports::TeleportTo(app::Vector3 position, bool shouldInterpolate)
	{
		auto &manager = game::EntityManager::instance();
		auto avatar = manager.avatar();
		if (avatar->moveComponent() == nullptr)
		{
			LOG_ERROR("Avatar has no move component, Is scene loaded?");
			return;
		}
		if (shouldInterpolate)
		{
			float speed = f_Speed;
			auto avatarPos = manager.avatar()->absolutePosition();
			auto endPos = position;
			std::thread interpolate([avatarPos, endPos, &manager, speed]()
									{
                            float t = 0.0f;
							app::Vector3 zero = {0,0,0};
							auto newPos = zero;
                            while (t < 1.0f) {
                                newPos = app::Vector3_Lerp(avatarPos, endPos, t, nullptr);
                                manager.avatar()->setAbsolutePosition(newPos);
								t += speed / 100.0f;
                                Sleep(10); 
                            } });
			interpolate.detach();
		}
		else
		{
			if (PositionDistance(position, app::ActorUtils_GetAvatarPos(nullptr)) > 60.0f)
				MapTeleport::GetInstance().TeleportTo(position);
			else
				manager.avatar()->setAbsolutePosition(position);
		}
	}

	void CustomTeleports::OnTeleportKeyPressed(bool next)
	{
		if (!f_Enabled->enabled() || !selectedIndex)
			return;

		app::Vector3 position;

		if (selectedByClick)
		{
			position = Teleports.at(*selectedIndex).position;
			selectedByClick = false;
		}
		else
		{
			std::vector list(checkedIndices.begin(), checkedIndices.end());
			if (next ? selectedIndex == list.back() : selectedIndex == list.front())
				return;

			auto index = std::distance(list.begin(), std::find(list.begin(), list.end(), *selectedIndex));
			selectedIndex = list.at(index + (next ? 1 : -1));
			position = Teleports.at(*selectedIndex).position;
		}
		TeleportTo(position, f_Interpolate->enabled());
		UpdateIndexName();
	}

	void CustomTeleports::OnPrevious()
	{
		if (f_Auto->enabled()) return;
		OnTeleportKeyPressed(false);
	}
	void CustomTeleports::OnNext()
	{
		if (f_Auto->enabled()) return;
		OnTeleportKeyPressed(true);
	}

	void CustomTeleports::OnGameUpdate()
	{
		if (!f_Enabled->enabled() || !f_Auto->enabled())
			return;

		auto currentTime = util::GetCurrentTimeMillisec();
		if (currentTime < nextTime)
			return;

		auto loadingManager = GET_SINGLETON(MoleMole_LoadingManager);
		if (loadingManager == nullptr || !app::MoleMole_LoadingManager_IsLoaded(loadingManager, nullptr))
			return;

		auto camera = app::Camera_get_main(nullptr);
		if (camera == nullptr) return;

		if (!app::Behaviour_get_isActiveAndEnabled(reinterpret_cast<app::Behaviour*>(camera), nullptr))
			return;

		nextTime = currentTime + (int64_t)f_DelayTime * 1000;
		OnTeleportKeyPressed(true); 
	}

	void itr(std::regex exp, std::string name, std::string s)
	{
		std::sregex_iterator itr(name.begin(), name.end(), exp);
		while (itr != std::sregex_iterator())
		{
			for (unsigned i = 0; i < itr->size(); i++)
				s.append((*itr)[i]);
			itr++;
		}
	}

	void CustomTeleports::UpdateIndexName()
	{
		// abbreviate teleport names that are too long
		std::string name(!selectedIndex || checkedIndices.empty() ? "" : Teleports.at(*selectedIndex).name);
		if (name.length() > 15)
		{
			std::string shortened;
			std::regex numsExp("[\\d]+");
			std::regex firstCharsExp("\\b[A-Za-z]");
			itr(firstCharsExp, name, shortened);
			itr(numsExp, name, shortened);
			name = shortened;
		}
		selectedIndexName = name;
	}

	void CustomTeleports::DrawMain()
	{
		// Buffers
		static std::string nameBuffer_;
		static std::string searchBuffer_;
		static std::string JSONBuffer_;
		static std::string descriptionBuffer_;

		ImGui::InputText(_TR("Name"), &nameBuffer_);
		ImGui::InputText(_TR("Description"), &descriptionBuffer_);
		if (ImGui::Button(_TR("Add Teleport")))
		{
			selectedIndex = std::nullopt;
			UpdateIndexName();
			SerializeTeleport(Teleport_(nameBuffer_, app::ActorUtils_GetAvatarPos(nullptr), descriptionBuffer_));
			nameBuffer_ = "";
			descriptionBuffer_ = "";
		}
		ImGui::SameLine();

		if (ImGui::Button(_TR("Reload")))
		{
			selectedIndex = std::nullopt;
			UpdateIndexName();
			checkedIndices.clear();
			ReloadTeleports();
		}

		ImGui::SameLine();
		if (ImGui::Button(_TR("Open Folder")))
		{
			CheckFolder();
			ShellExecuteA(NULL, "open", dir.string().c_str(), NULL, NULL, SW_SHOW);
		}

		ImGui::SameLine();
		if (ImGui::Button(_TR("Load from JSON")))
		{
			if (!JSONBuffer_.empty()) {
				auto t = SerializeFromJson(JSONBuffer_, false);
				if (t.has_value()) {
					selectedIndex = std::nullopt;
					UpdateIndexName();
					SerializeTeleport(t.value());
				}
				JSONBuffer_.clear();
			}

		}
		ImGui::InputTextMultiline(_TR("JSON input"), &JSONBuffer_, ImVec2(0, 50), ImGuiInputTextFlags_AllowTabInput);

		ConfigWidget(_TR("Teleport Next"), f_Next, true, _TR("Press to teleport next of selected."));
		ConfigWidget(_TR("Teleport Previous"), f_Previous, true, _TR("Press to teleport previous of selected."));
		ConfigWidget(_TR("Enable"), f_Enabled,
					 _TR("Enable teleport-through-list functionality.\n"
					 "Usage:\n"
					 "1. Put Checkmark to the teleports you want to teleport using hotkey\n"
					 "2. Single click the teleport (with checkmark) to select where you want to start\n"
					 "3. You can now press Next or Previous Hotkey to Teleport through the Checklist\n"
					 "Initially it will teleport the player to the selection made\n"
					 "Note: Double click or click the arrow to open teleport details"));
		ConfigWidget(_TR("Enable Interpolation"), f_Interpolate, _TR("Enable interpolation between teleports when using keybinds."));
		ImGui::SetNextItemWidth(300.0f);
		ConfigWidget(_TR("Interpolation Speed"), f_Speed, 0.1f, 0.1f, 99.0f,
					 _TR("Interpolation speed.\n recommended setting below or equal to 0.1."));
		ConfigWidget(_TR("Auto Teleport"), f_Auto, _TR("Enable automatic forward teleporation between teleports"));
		ImGui::SetNextItemWidth(300.0f);
		ConfigWidget(_TR("Delay Time (s)"), f_DelayTime, 1, 0, 60, _TR("Delay (in s) between teleport.\n"
			"Note: This is not fully tested detection-wise.\nNot recommended with low values."));

		if (ImGui::Button(_TR("Delete Checked")))
		{
			if (!Teleports.empty())
			{
				if (checkedIndices.empty())
				{
					LOG_INFO("No teleports selected");
					return;
				}

				std::vector<std::string> teleportNames;
				for (auto &Teleport : Teleports)
					teleportNames.push_back(Teleport.name);

				for (const auto index : checkedIndices)
				{
					std::filesystem::remove(dir / (teleportNames[index] + ".json"));
					LOG_INFO("Deleted teleport %s", teleportNames[index].c_str());
				}
				checkedIndices.clear();
				UpdateIndexName();
				ReloadTeleports();
			} 
			else 
			{
				LOG_INFO("No teleports to delete");
			}
		}
		ImGui::SameLine();
		HelpMarker(_TR("Warning: This will delete the file from the directory and\n \
			remove the teleport from the list. It will be lost forever."));

		if (ImGui::TreeNode(_TR("Teleports")))
		{
			std::sort(Teleports.begin(), Teleports.end(), [](const auto &a, const auto &b)
					  { return StrCmpLogicalW(std::wstring(a.name.begin(), a.name.end()).c_str(), std::wstring(b.name.begin(), b.name.end()).c_str()) < 0; });
			bool allSearchChecked = std::includes(checkedIndices.begin(), checkedIndices.end(), searchIndices.begin(), searchIndices.end()) && !searchIndices.empty();
			bool allChecked = (checkedIndices.size() == Teleports.size() && !Teleports.empty()) || allSearchChecked;
			ImGui::Checkbox(_TR("All"), &allChecked);
			if (ImGui::IsItemClicked())
			{
				if (!Teleports.empty())
				{
					if (allChecked)
					{
						selectedIndex = std::nullopt;
						if (!searchIndices.empty())
							for (const auto i : searchIndices)
								checkedIndices.erase(i);
						else
							checkedIndices.clear();
					}
					else if (!searchIndices.empty())
						checkedIndices.insert(searchIndices.begin(), searchIndices.end());
					else
						for (size_t i = 0; i < Teleports.size(); i++)
							checkedIndices.insert(i);
					UpdateIndexName();
				}
			}
			ImGui::SameLine();
			ImGui::InputText(_TR("Search"), &searchBuffer_);
			uint64_t index = 0;
			searchIndices.clear();

			size_t maxNameLength = 0;
			for (auto &Teleport : Teleports)
				if (Teleport.name.length() > maxNameLength)
					maxNameLength = Teleport.name.length();
			ImGui::BeginTable("Teleports", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings);
			ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 20);
			ImGui::TableSetupColumn(_TR("Commands"), ImGuiTableColumnFlags_WidthFixed, 130);
			ImGui::TableSetupColumn(_TR("Name"), ImGuiTableColumnFlags_WidthFixed, static_cast<float>(maxNameLength) * 8 + 10);
			ImGui::TableSetupColumn(_TR("Position"));
			ImGui::TableHeadersRow();

			for (const auto &[name, position, description] : Teleports)
			{
				if (searchBuffer_.empty() || std::search(name.begin(), name.end(), searchBuffer_.begin(), searchBuffer_.end(), [](char a, char b)
														 { return std::tolower(a) == std::tolower(b); }) != name.end())
				{
					if (!searchBuffer_.empty())
						searchIndices.insert(index);
					bool checked = std::any_of(checkedIndices.begin(), checkedIndices.end(), [&index](const auto i)
											   { return i == index; });
					bool isSelected = index == selectedIndex;
					std::string stringIndex = std::to_string(index);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text(_TR("%d"), index);
					ImGui::TableNextColumn();
					ImGui::Checkbox(("##Index" + stringIndex).c_str(), &checked);
					if (ImGui::IsItemClicked(0))
					{
						if (checked)
						{
							if (isSelected)
								selectedIndex = std::nullopt;
							checkedIndices.erase(index);
						}
						else
							checkedIndices.insert(index);
						UpdateIndexName();
					}

					ImGui::SameLine();
					if (ImGui::Button((_TR("TP##Button") + stringIndex).c_str()))
					{
						TeleportTo(position, false);
					}

					ImGui::SameLine();
					if (ImGui::Button((_TR("Lerp##Button") + stringIndex).c_str()))
					{
						TeleportTo(position, true);
					}
					ImGui::SameLine();

					if (ImGui::Button((_TR("Select##Button") + stringIndex).c_str()))
					{
						auto isChecked = checkedIndices.find(index) != checkedIndices.end();
						if (isChecked) {
							selectedIndex = index;
							selectedByClick = true;
							UpdateIndexName();
						}
					}
					ImGui::TableNextColumn();

					ImGui::PushStyleColor(ImGuiCol_Text, isSelected ? ImGui::GetColorU32(ImGuiCol_CheckMark) : ImGui::GetColorU32(ImGuiCol_Text));
					ImGui::Text("%s", name.c_str());
					ImGui::PopStyleColor();
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("%s", description.c_str());
						ImGui::Text(_TR("Distance: %.2f"), PositionDistance(position, app::ActorUtils_GetAvatarPos(nullptr)));
						ImGui::EndTooltip();
					}
					ImGui::TableNextColumn();
					ImGui::Text("%f, %f, %f", position.x, position.y, position.z);
				}
				index++;
			}
			ImGui::EndTable();
			ImGui::TreePop();
		}

		if (selectedIndex)
			ImGui::Text("%s: [%llu] %s", _TR("Selected"), *selectedIndex, Teleports[*selectedIndex].name.c_str());
	}

	bool CustomTeleports::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}

	void CustomTeleports::DrawStatus()
	{
		ImGui::Text("%s\n[%s|%s]", _TR("Custom Teleport"), f_Auto->enabled() ? _TR("Auto") : _TR("Manual"), selectedIndexName);
	}

	CustomTeleports &CustomTeleports::GetInstance()
	{
		static CustomTeleports instance;
		return instance;
	}
}
