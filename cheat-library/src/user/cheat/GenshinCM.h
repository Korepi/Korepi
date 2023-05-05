#pragma once

#include <cheat-base/cheat/CheatManagerBase.h>

namespace cheat
{
	namespace internal
	{
		struct AccountData
		{
		public:
			uint32_t userID;
			std::string nickName;

			AccountData() : userID(0), nickName() {}
		};

		struct AccountConfig
		{
		public:

			std::unordered_map<uint32_t, std::string> pseudos;
			std::unordered_map<uint32_t, AccountData> accounts;
			std::unordered_map<uint32_t, std::string> id2Profiles;
			std::unordered_map<std::string, std::unordered_set<uint32_t>> profiles2id;

			inline bool operator==(const AccountConfig& other)
			{
				return accounts.size() == other.accounts.size() && pseudos.size() == other.pseudos.size();
			}

			AccountConfig() : pseudos(), accounts(), id2Profiles() { }
		};

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AccountData, nickName, userID)

		inline void to_json(nlohmann::json& nlohmann_json_j, const AccountConfig& nlohmann_json_t)
		{
			NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, accounts, pseudos, id2Profiles))
		}

		inline void from_json(const nlohmann::json& nlohmann_json_j, AccountConfig& nlohmann_json_t)
		{
			NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, accounts, pseudos, id2Profiles))

			for (auto& [userID, profileName] : nlohmann_json_t.id2Profiles)
			{
				auto& profileIDs = nlohmann_json_t.profiles2id[profileName];
				profileIDs.insert(userID);
			}
		}

	}


	class GenshinCM : public CheatManagerBase
	{
	public:
		static GenshinCM& instance();

		void CursorSetVisibility(bool visibility) final;
		bool CursorGetVisibility() final;

	protected:
		enum class ShowType
		{
			Pseudo,
			UserID,
			Nickname
		};

		config::Field<config::Enum<ShowType>> f_ShowType;

		internal::AccountData m_CurrentAccount;
		config::Field<internal::AccountConfig> f_AccConfig;

		GenshinCM();

		bool IsAccountAttached(uint32_t userID, const std::string& profileName);
		std::string GetAccountDisplayName(uint32_t uid);

		void DetachAccount(uint32_t uid, const std::string& profileName);
		void AttachAccount(uint32_t uid, const std::string& profileName);

		void DrawProfileEntryActivities(const std::string& profileName) final;
		void DrawProfileEntry(const std::string& profileName) final;
		void DrawProfileTableHeader() final;
		int GetProfileTableColumnCount() final;
		void DrawAccountsList(const std::string& profileName);
		void DrawProfileConfiguration() final;
		void DrawProfileLine() final;
		void DrawPseudoRename(uint32_t userID);

		void OnAccountChanged(uint32_t userID);
	};
}
