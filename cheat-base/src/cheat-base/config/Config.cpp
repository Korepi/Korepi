#include <pch.h>
#include "config.h"

#include <atomic>
#include <fstream>

#include <SimpleIni.h>
#include <cheat-base/util.h>

namespace config
{
	TEvent<> ProfileChanged;

	static std::filesystem::path s_Filepath;
	static nlohmann::json s_ConfigRoot;
	static nlohmann::json s_EmptyJObject = nlohmann::json::object();

	// Little speed-up
	static nlohmann::json* s_ProfileRoot = nullptr;
	static nlohmann::json* s_Profiles = nullptr;
	static nlohmann::json* s_SharedRoot = nullptr;

	static std::mutex s_ProfileMutex;
	static std::string s_ProfileName;
	static std::vector<std::string> s_ProfilesNames;

	static const int c_SaveDelay = 2000;
	static TEvent<>* s_UpdateEvent = nullptr;
	static std::atomic<int64_t> s_NextSaveTimestamp = 0;

	static std::vector<std::shared_ptr<internal::FieldEntry>> s_Entries;

	void LoadFile()
	{
		std::ifstream fileInput(s_Filepath, std::ios::in);
		if (!fileInput.is_open())
		{
			LOG_DEBUG("Failed to open config file, maybe it's first launch.");
			return;
		}

		try
		{
			s_ConfigRoot = nlohmann::json::parse(fileInput);
		}
		catch (nlohmann::json::parse_error& ex)
		{
			LOG_ERROR("Parse error at byte %llu", ex.byte);
			return;
		}
	}

	void UpdateProfilesNames()
	{
		std::lock_guard _lock(s_ProfileMutex);
		s_ProfilesNames.clear();
		for (auto& [name, _] : s_Profiles->items())
		{
			s_ProfilesNames.push_back(name);
		}
	}

	void Initialize(const std::string& filePath)
	{
		s_ConfigRoot = {};
		s_Filepath = filePath;
		LoadFile();

		if (!s_ConfigRoot.contains("current_profile"))
		{
			s_ConfigRoot = {
				{ "shared", {} },
				{ "profiles", {} },
				{ "current_profile", ""}
			};
		}

		s_Profiles = &s_ConfigRoot["profiles"];
		s_SharedRoot = &s_ConfigRoot["shared"];

		if (s_ConfigRoot["current_profile"] == "")
			CreateProfile("default");
		else
			ChangeProfile(s_ConfigRoot["current_profile"]);

		UpdateProfilesNames();
	}

	void OnUpdate();

	void SetupUpdate(TEvent<>* updateEvent)
	{
		s_UpdateEvent = updateEvent;
		(*s_UpdateEvent) += FUNCTION_HANDLER(OnUpdate);
	}

	void UpdateSaveTimestamp()
	{
		if (!s_UpdateEvent)
			return;

		if (s_NextSaveTimestamp != 0)
			return;

		s_NextSaveTimestamp = util::GetCurrentTimeMillisec() + c_SaveDelay;
	}

	void ResetNotShared()
	{
		for (auto& entry : s_Entries)
		{
			if (!entry->IsShared())
				entry->Reset();
		}
	}

	nlohmann::json& GetFieldJsonContainer(internal::FieldEntry* field, bool create = false)
	{
		if (field->GetContainer() != nullptr)
			return *field->GetContainer();

		nlohmann::json* rootContainer = s_ProfileRoot;
		if (field->IsShared())
			rootContainer = s_SharedRoot;

		auto sectionParts = util::StringSplit("::", field->GetSection());
		for (auto& part : sectionParts)
		{
			if (!rootContainer->contains(part))
			{
				if (!create)
					return s_EmptyJObject;

				(*rootContainer)[part] = {};
			}

			rootContainer = &(*rootContainer)[part];
		}

		auto& sectionContainer = *rootContainer;
		if (!sectionContainer.contains(field->GetName()))
		{
			if (!create)
				return s_EmptyJObject;

			sectionContainer[field->GetName()] = {};
		}

		auto& fieldContainer = sectionContainer[field->GetName()];
		field->SetContainer(&fieldContainer);
		return fieldContainer;
	}

	void RemoveFieldContainer(internal::FieldEntry* field, const std::string& section, const std::string& name, bool shared)
	{
		field->SetContainer(nullptr);

		nlohmann::json* rootContainer = s_ProfileRoot;
		if (shared)
			rootContainer = s_SharedRoot;

		auto sectionParts = util::StringSplit("::", section);
		std::list<std::pair<std::string, nlohmann::json*>> nodePath;
		for (auto& part : sectionParts)
		{
			if (!(*rootContainer).contains(part))
				return;

			nodePath.push_front({ part, rootContainer });
			rootContainer = &(*rootContainer)[part];
		}

		if (!rootContainer->contains(name))
			return;

		rootContainer->erase(name);
		for (auto& [key, node] : nodePath)
		{
			if (!(*node)[key].empty())
				break;

			node->erase(key);
		}
	}

	void UpdateField(internal::FieldEntry* field)
	{
		auto& fieldContainer = GetFieldJsonContainer(field);
		field->FromJson(fieldContainer);
	}

	void UpdateNotShared()
	{
		ResetNotShared();
		for (auto& entry : s_Entries)
		{
			if (!entry->IsShared())
				UpdateField(entry.get());
		}
	}

	void LoadField(internal::FieldEntry* field)
	{
		auto& fieldContainer = GetFieldJsonContainer(field, true);

		auto jObject = field->ToJson();
		if (jObject.empty())
			RemoveFieldContainer(field, field->GetSection(), field->GetName(), field->IsShared());
		else
			fieldContainer = jObject;
	}

	void LoadAll()
	{
		for (auto& entry : s_Entries)
		{
			LoadField(entry.get());
		}
	}

	void OnFieldChanged(internal::FieldEntry* field)
	{
		LoadField(field);
		Save();
	}

	void OnFieldMoved(internal::FieldEntry* field, const std::string& oldSection, bool oldShared)
	{
		RemoveFieldContainer(field, oldSection, field->GetName(), oldShared);
		OnFieldChanged(field);
	}

	void OnFieldReposition(internal::FieldEntry* field, const std::string& oldSection, bool oldShared)
	{
		field->SetContainer(nullptr);
		UpdateField(field);
	}

	void internal::AddField(std::shared_ptr<FieldEntry> field)
	{
		s_Entries.push_back(field);
		UpdateField(field.get());
		field->ChangedEvent += FUNCTION_HANDLER(OnFieldChanged);
		field->MovedEvent += FUNCTION_HANDLER(OnFieldMoved);
		field->RepositionEvent += FUNCTION_HANDLER(OnFieldReposition);
	}

	void Refresh()
	{
		LoadAll();
		Save();
	}

	void SaveInternal()
	{
		std::ofstream fileOutput(s_Filepath, std::ios::out);
		if (!fileOutput.is_open())
		{
			LOG_DEBUG("Failed to open config file for writing.");
			UpdateSaveTimestamp();
			return;
		}

		fileOutput << s_ConfigRoot.dump(4);
		fileOutput.close();
	}

	void Save()
	{
		if (s_UpdateEvent)
		{
			UpdateSaveTimestamp();
			return;
		}
		SaveInternal();
	}

	void OnUpdate()
	{
		if (s_NextSaveTimestamp > 0 && util::GetCurrentTimeMillisec() > s_NextSaveTimestamp)
		{
			s_NextSaveTimestamp = 0;
			SaveInternal();
		}
	}

	void CreateProfile(const std::string& profileName, bool moveAfterCreate)
	{
		if (s_Profiles->contains(profileName))
		{
			if (moveAfterCreate)
				ChangeProfile(profileName);
			return;
		}

		(*s_Profiles)[profileName] = {};
		UpdateProfilesNames();

		if (moveAfterCreate)
			ChangeProfile(profileName);
		Save();
	}

	void RemoveProfile(const std::string& profileName)
	{
		if (!s_Profiles->contains(profileName))
			return;

		if (s_Profiles->size() == 1)
			return;

		if (s_ProfileName == profileName)
		{
			for (auto& [name, value] : s_Profiles->items())
			{
				if (name != profileName)
				{
					ChangeProfile(name);
					break;
				}
			}
		}

		s_Profiles->erase(profileName);
		UpdateProfilesNames();
		Save();
	}

	void RenameProfile(const std::string& oldProfileName, const std::string& newProfileName)
	{
		if (!s_Profiles->contains(oldProfileName) || s_Profiles->contains(newProfileName))
			return;

		if (s_ProfileName == oldProfileName)
			s_ProfileRoot = nullptr;

		(*s_Profiles)[newProfileName] = (*s_Profiles)[oldProfileName];
		s_Profiles->erase(oldProfileName);

		if (s_ProfileRoot == nullptr)
		{
			for (auto& entry : s_Entries)
			{
				if (!entry->IsShared())
					entry->SetContainer(nullptr);
			}

			ChangeProfile(newProfileName);
		}
		UpdateProfilesNames();
		Save();
	}

	void ChangeProfile(const std::string& profileName)
	{
		if (s_ProfileName == profileName)
			return;

		if (!s_Profiles->contains(profileName))
			return;

		std::lock_guard _lock(s_ProfileMutex);

		s_ProfileRoot = &(*s_Profiles)[profileName];
		s_ProfileName = profileName;
		
		s_ConfigRoot["current_profile"] = profileName;
		UpdateNotShared();
		Save();

		ProfileChanged();
	}

	void DuplicateProfile(const std::string& profileName)
	{
		// Find a unique name for the new profile
		uint32_t counter = 0;
		std::ostringstream buffer;
		std::string newProfileName;
		do
		{
			buffer.str(std::string());
			buffer.clear();
			counter++;
			buffer << profileName << " (" << counter << ")";
			newProfileName = buffer.str();
		} while (s_Profiles->contains(newProfileName));

		// nlohmann::json copy constructor will take care of duplicating
		(*s_Profiles)[newProfileName] = (*s_Profiles)[profileName];
		UpdateProfilesNames();
		Save();
	}

	std::vector<std::string> const& GetProfiles()
	{
		return s_ProfilesNames;
	}

	std::string const& CurrentProfileName()
	{
		return s_ProfileName;
	}
}