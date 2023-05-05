#pragma once

#include <il2cpp-appdata.h>
#include <vector>

#include <helpers.h>

#include "Entity.h"
#include "SimpleFilter.h"

namespace cheat::game
{
	struct WaypointInfo
	{
		uint32_t sceneId = 0;
		uint32_t waypointId = 0;
		app::Vector3 position = {};
		app::MapModule_ScenePointData* data = nullptr;
	};

	WaypointInfo FindNearestWaypoint(app::Vector3& position, uint32_t targetId = 0);
	static std::vector<WaypointInfo> GetUnlockedWaypoints(uint32_t targetId = 0);
	app::Vector3 GetWaypointPosition(uint32_t targetSceneId, uint32_t targetWaypointId);
	bool IsWaypointTeleportable(app::ConfigScenePoint* waypointConfig);
	uint32_t GetCurrentMapSceneID();
	uint32_t GetCurrentPlayerSceneID();

	app::GadgetEntity* GetGadget(uint32_t runtimeID);
	app::GadgetEntity* GetGadget(Entity* entity);
	app::GadgetEntity* GetGadget(app::BaseEntity* entity);

	game::Entity* FindNearestEntity(const IEntityFilter& filter);
	game::Entity* FindNearestEntity(std::vector<game::Entity*>& entities);

	app::AccountDataItem* GetAccountData();

	bool IsInGame();

	void ShowInGameNormalMessage(std::string content, app::Color bgColor = app::Color(255,255,255,255), bool showBackground = false);
	void ShowInGameInfoMessage(std::string content, bool discardIfDuplicated = false, app::UIShowPriority__Enum priority = app::UIShowPriority__Enum::Normal);
	void ShowInGameGeneralDialog(std::string title, std::string content, app::GeneralDialogContext_GeneralDialogType__Enum dialogType = app::GeneralDialogContext_GeneralDialogType__Enum::DOUBLE_BUTTON, app::UIType__Enum uiType = app::UIType__Enum::Dialog, app::UIShowPriority__Enum mode = app::UIShowPriority__Enum::Normal, bool showNormalCloseBtn = true, bool isToggleDialog = false);
	void ShowInGameSimpleGeneralDialog(std::string title, std::string content);
    void ShowInGameNormalMessage(std::wstring content, app::Color bgColor = app::Color(255, 255, 255, 255), bool showBackground = false);
    void ShowInGameInfoMessage(std::wstring content, bool discardIfDuplicated = false, app::UIShowPriority__Enum priority = app::UIShowPriority__Enum::Normal);
    void ShowInGameGeneralDialog(std::wstring title, std::wstring content, app::GeneralDialogContext_GeneralDialogType__Enum dialogType = app::GeneralDialogContext_GeneralDialogType__Enum::DOUBLE_BUTTON, app::UIType__Enum uiType = app::UIType__Enum::Dialog, app::UIShowPriority__Enum mode = app::UIShowPriority__Enum::Normal, bool showNormalCloseBtn = true, bool isToggleDialog = false);
    void ShowInGameSimpleGeneralDialog(std::wstring title, std::wstring content);
}
