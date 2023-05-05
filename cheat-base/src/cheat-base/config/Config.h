#pragma once

#include "Field.h"
#include <vector>
#include <string>
#include <filesystem>

#include "fields/Toggle.h"
#include "fields/Enum.h"

#define SNFEX(field, name, section, defaultValue, shared) config::CreateField<decltype(##field##)::_ValueType>(name, section, shared, defaultValue)
#define SNFB(field, section, defaultValue, shared) SNFEX(field, config::internal::FixFieldName(#field), section, defaultValue, shared)
#define SNF(field, section, defaultValue) SNFB(field, section, defaultValue, false)

#define NFEX(field, name, section, defaultValue, shared) field##(SNFEX(field, name, section, defaultValue, shared))
#define NFEXUP(field, name, section, shared, ...) field##(config::CreateField<decltype(##field##)::_ValueType>(name, section, shared, __VA_ARGS__))

#define NFB(field, section, defaultValue, shared) NFEX(field, config::internal::FixFieldName(#field), section, defaultValue, shared)
#define NFS(field, section, defaultValue) NFB(field, section, defaultValue, true)
#define NF(field, section, defaultValue) NFB(field, section, defaultValue, false)

#define NFPB(field, section, shared, ...) NFEXUP(field, config::internal::FixFieldName(#field), section, shared, __VA_ARGS__)
#define NFPS(field, section, ...) NFPB(field, section, true, __VA_ARGS__)
#define NFP(field, section, ...) NFPB(field, section, false, __VA_ARGS__)

namespace config
{
	namespace internal
	{
		template<typename T>
		std::vector<T> s_Fields;

		void AddField(std::shared_ptr<FieldEntry> field);

		inline std::string FixFieldName(const std::string& fieldName)
		{
			if (fieldName.substr(1, 1) == "_")
				return fieldName.substr(2);
			return fieldName;
		}
	}

	template<typename T, typename... Args>
	Field<T> CreateField(const std::string& name, const std::string& section, bool multiProfile, Args... args)
	{
		auto newField = Field<T>(name, section, T(args...), multiProfile);
		internal::s_Fields<Field<T>>.push_back(newField);
		internal::AddField(newField.entry());
		return newField;
	}

	template<typename T>
	std::vector<Field<T>>& GetFields()
	{
		return internal::s_Fields<Field<T>>;
	}

	void Initialize(const std::string& filePath);
	void SetupUpdate(TEvent<>*);

	void Refresh();
	void Save();

	void CreateProfile(const std::string& profileName, bool moveAfterCreate = true);
	void RemoveProfile(const std::string& profileName);
	void RenameProfile(const std::string& oldProfileName, const std::string& newProfileName);
	void ChangeProfile(const std::string& profileName);
	void DuplicateProfile(const std::string& profileName);
	std::vector<std::string> const& GetProfiles();
	std::string const& CurrentProfileName();

	extern TEvent<> ProfileChanged;
}