#include "pch-il2cpp.h"
#include "RSAPatch.h"
#include <helpers.h>

namespace cheat::feature
{
	static void RSA_FromXmlString_Hook(app::RSA *__this, app::String *xmlString, MethodInfo *method);

	RSAPatch::RSAPatch() : Feature(),
		NFP(f_Enabled, "General::RSAPatch", "RSA Patch", false),
		NF(f_PublicKey, "General::RSAPatch", false),
		NF(f_PrivateKey, "General::RSAPatch", false)
	{
		HookManager::install(app::RSA_FromXmlString, RSA_FromXmlString_Hook);
	}

	const FeatureGUIInfo &RSAPatch::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("RSA Patch", "Settings");
		return info;
	}

	void RSAPatch::DrawMain()
	{
		ConfigWidget(_TR("Enable RSA Patch"), f_Enabled, _TR("Runtime patcher. Only use it on private servers."));
		ConfigWidget(_TR("Custom Public Key"), f_PublicKey, _TR("If disabled, use grasscutter's public RSA key."));
		ConfigWidget(_TR("Custom Private Key"), f_PrivateKey, _TR("If disabled, don't patch private RSA key"));
	}

	bool RSAPatch::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}
	
	void RSAPatch::DrawStatus()
	{
		ImGui::Text(_TR("RSA Patch"));
		// to be done
	}

	RSAPatch &RSAPatch::GetInstance()
	{
		static RSAPatch instance;
		return instance;
	}

	static void RSA_FromXmlString_Hook(app::RSA *__this, app::String *xmlString, MethodInfo *method)
	{
		auto &rsaPatch = RSAPatch::GetInstance();
		auto key = il2cppi_to_string(xmlString);

		if (rsaPatch.f_Enabled->enabled())
		{
			app::String *public_key;
			app::String *private_key;

			if (rsaPatch.f_PublicKey->enabled())
				public_key = string_to_il2cppi(rsaPatch.f_PublicKey->value());
			else // Use Grasscutter Public key
				public_key = string_to_il2cppi("<RSAKeyValue><Modulus>xbbx2m1feHyrQ7jP+8mtDF/pyYLrJWKWAdEv3wZrOtjOZzeLGPzsmkcgncgoRhX4dT+1itSMR9j9m0/OwsH2UoF6U32LxCOQWQD1AMgIZjAkJeJvFTrtn8fMQ1701CkbaLTVIjRMlTw8kNXvNA/A9UatoiDmi4TFG6mrxTKZpIcTInvPEpkK2A7Qsp1E4skFK8jmysy7uRhMaYHtPTsBvxP0zn3lhKB3W+HTqpneewXWHjCDfL7Nbby91jbz5EKPZXWLuhXIvR1Cu4tiruorwXJxmXaP1HQZonytECNU/UOzP6GNLdq0eFDE4b04Wjp396551G99YiFP2nqHVJ5OMQ==</Modulus><Exponent>AQAB</Exponent></RSAKeyValue>");

			// Private key contains: <P> <Q> <DP> <DQ> <D> <Exponent> <Modulus> <InverseQ>
			// Public key contains: <Exponent> <Modulus>

			// Public key
			if ((key.rfind("<RSAKeyValue><Modulus>15RBm/vARY0axYksImhsTicpv09OYfS4+wCvmE7") != std::string::npos || key.rfind("<RSAKeyValue><Modulus>lCwdYrveozYYcKOSz4cjBfORvd6POZSxsM9JybW") != std::string::npos)) // 2nd time fn is called
			{
				memcpy(xmlString, public_key, public_key->fields.length * 2);
				LOG_INFO("Public Key Patched.");
			}

			// Private key: no need to patch on grasscutter
			if (rsaPatch.f_PrivateKey->enabled())
			{
				private_key = string_to_il2cppi(rsaPatch.f_PrivateKey->value());

				if (key.find("</DP>") != std::string::npos && key.rfind("</InverseQ>") != std::string::npos)
				{
					memcpy(xmlString, private_key, private_key->fields.length * 2);
					LOG_INFO("Private Key Patched.");
				}
			}
		}
		CALL_ORIGIN(RSA_FromXmlString_Hook, __this, xmlString, method);
	}

}