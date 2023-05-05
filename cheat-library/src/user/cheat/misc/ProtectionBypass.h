#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>
#include <map>

#include <il2cpp-appdata.h>

namespace cheat::feature 
{

	class ProtectionBypass : public Feature
    {
	public:
		config::Field<bool> f_Enabled;
		config::Field<bool> f_SpoofLuaShell;

		bool report_sent = false;

		static ProtectionBypass& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;
	
		app::Byte__Array* OnRecordUserData(int32_t nType);
		void OnReportLuaShell(void* __this, app::String* type, app::String* value, MethodInfo* method);

		void Init();

	private:
		ProtectionBypass();
		std::map<int32_t, std::string> m_CorrectSignatures;
	};
}

