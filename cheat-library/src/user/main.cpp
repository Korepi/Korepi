#include "pch-il2cpp.h"
#include "main.h"

#include <helpers.h>
#include <il2cpp-init.h>
#include <cheat/cheat.h>
#include <cheat-base/cheat/misc/Settings.h>

#include <tlhelp32.h>
#include <cheat/ILPatternScanner.h>
#include <resource.h>
#include <cheat/debugger.h>

void Run(HMODULE* phModule)
{
	ResourceLoader::SetModuleHandle(*phModule);
	util::SetCurrentPath(util::GetModulePath(*phModule));

	// Init config
	config::Initialize((util::GetCurrentPath() / "cfg.json").string());

	// Init logger
	auto& settings = cheat::feature::Settings::GetInstance();
	if (settings.f_FileLogging)
	{
		Logger::PrepareFileLogging((util::GetCurrentPath() / "logs").string());
		Logger::SetLevel(Logger::Level::Trace, Logger::LoggerType::FileLogger);
	}

	if (settings.f_ConsoleLogging)
	{
		Logger::SetLevel(Logger::Level::Debug, Logger::LoggerType::ConsoleLogger);
		il2cppi_new_console();
	}

	DebuggerBypassPre();

	while (GetModuleHandle("UserAssembly.dll") == nullptr)
	{
		LOG_DEBUG("UserAssembly.dll isn't initialized, waiting for 2 sec.");
		Sleep(2000);
	}

	if (!settings.initializationSkip)
	{
#ifdef _DEBUG
		LOG_DEBUG("Waiting 10sec for loading game library.");
		Sleep(15000);
#else
		uint32_t delay = settings.f_InitializationDelay.value();
		LOG_DEBUG("Waiting %.3f sec for game to initialize.\n(If Genshin crashes after this line you might want to try longer delay in cfg.ini)", (float)delay / 1000);
		Sleep(delay);
#endif
	}

	DebuggerBypassPost();

	init_il2cpp();

	cheat::Init();

    LOG_DEBUG("Config path is at %s", (util::GetCurrentPath() / "cfg.json").string().c_str());
}