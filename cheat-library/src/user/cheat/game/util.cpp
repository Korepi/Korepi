#include <pch-il2cpp.h>
#include "util.h"

#include <helpers.h>
#include "EntityManager.h"

namespace cheat::game
{
	std::vector<WaypointInfo> GetUnlockedWaypoints(uint32_t targetSceneId)
	{
		auto mapModule = GET_SINGLETON(MoleMole_MapModule);
		if (mapModule == nullptr)
			return {};

		if (targetSceneId == 0)
			targetSceneId = GetCurrentPlayerSceneID();

		auto result = std::vector<WaypointInfo>();

		auto waypointGroups = TO_UNI_DICT(mapModule->fields._scenePointDics, uint32_t, UniDict<uint32_t COMMA app::MapModule_ScenePointData>*);
		for (const auto& [sceneId, waypoints] : waypointGroups->pairs())
		{
			if (sceneId != targetSceneId)
				continue;

			for (const auto& [waypointId, waypoint] : waypoints->pairs())
			{
				if (waypoint.config == nullptr)
					continue;

				auto& config = waypoint.config->fields;
				uint16_t areaId = app::MoleMole_SimpleSafeUInt16_get_Value(config.areaIdRawNum, nullptr);
				bool isAreaUnlocked = app::MoleMole_MapModule_IsAreaUnlock(mapModule, sceneId, areaId, nullptr);

				if (waypoint.isUnlocked && isAreaUnlocked && !config._unlocked && !waypoint.isGroupLimit && !waypoint.isModelHidden && IsWaypointTeleportable(waypoint.config))
					result.push_back(WaypointInfo{ sceneId, waypointId, waypoint.config->fields._tranPos, (app::MapModule_ScenePointData*)&waypoint });
			}
		}
		return result;
	}

	// Finding nearest unlocked waypoint to the position
	WaypointInfo FindNearestWaypoint(app::Vector3& position, uint32_t targetSceneId)
	{
		float minDistance = -1;
		WaypointInfo result{};
		for (const auto& info : GetUnlockedWaypoints(targetSceneId)) {
			float distance = app::Vector3_Distance(position, info.position, nullptr);
			if (minDistance < 0 || distance < minDistance)
			{
				minDistance = distance;
				result = info;
			}
		}
		return result;
	}

	app::Vector3 GetWaypointPosition(uint32_t targetSceneId, uint32_t targetWaypointId)
	{
		app::Vector3 result{};

		auto mapModule = GET_SINGLETON(MoleMole_MapModule);
		if (mapModule == nullptr)
			return result;

		if(targetSceneId == 0)
		{
			auto mapManager = GET_SINGLETON(MoleMole_MapManager);
			if (mapManager == nullptr)
				targetSceneId = GetCurrentPlayerSceneID();
			else
				targetSceneId = mapManager->fields.mapSceneID;
		}

		auto waypointGroups = TO_UNI_DICT(mapModule->fields._scenePointDics, uint32_t, UniDict<uint32_t COMMA app::MapModule_ScenePointData>*);
		for (const auto& [sceneId, waypoints] : waypointGroups->pairs())
		{
			if (sceneId != targetSceneId)
				continue;

			for (const auto& [waypointId, waypoint] : waypoints->pairs())
			{
				if(waypointId != targetWaypointId)
					continue;
				if (waypoint.config == nullptr)
					return result;
				result = waypoint.config->fields._tranPos;
				if (result.x == 0 || result.y == 0 || result.z == 0)
					result = waypoint.config->fields._pos;
			}
		}

		return result;
	}

	bool IsWaypointTeleportable(app::ConfigScenePoint* waypointConfig)
	{
		if (!waypointConfig)
			return false;
		auto get_pointType = (app::RealScenePointType__Enum(*)(app::ConfigScenePoint*, MethodInfo*))waypointConfig->klass->vtable.get_pointType.methodPtr;
		auto get_pointType_MethodInfo = (MethodInfo*)waypointConfig->klass->vtable.get_pointType.method;
		auto pointType = get_pointType(waypointConfig, get_pointType_MethodInfo);
		return pointType != app::RealScenePointType__Enum::VehicleSummonPoint && pointType != app::RealScenePointType__Enum::SceneBuildingPoint;
	}

	uint32_t GetCurrentPlayerSceneID()
	{
		auto playerModule = GET_SINGLETON(MoleMole_PlayerModule);
		if (playerModule == nullptr)
			return 0;

		return playerModule->fields.curSceneID;
	}

	uint32_t GetCurrentMapSceneID()
	{
		auto mapManager = GET_SINGLETON(MoleMole_MapManager);
		if (mapManager == nullptr)
			return 0;

		return mapManager->fields.mapSceneID;
	}

	app::GadgetEntity* GetGadget(uint32_t runtimeID)
	{
		auto& manager = EntityManager::instance();
		return GetGadget(manager.entity(runtimeID));
	}

	app::GadgetEntity* GetGadget(app::BaseEntity* entity)
	{
		if (entity == nullptr)
			return nullptr;

		auto ent = Entity(entity);
		if (!ent.isGadget())
			return nullptr;

		return reinterpret_cast<app::GadgetEntity*>(entity);
	}

	app::GadgetEntity* GetGadget(Entity* entity)
	{
		if (entity == nullptr || !entity->isGadget())
			return nullptr;
		
		return reinterpret_cast<app::GadgetEntity*>(entity);
	}

	game::Entity* FindNearestEntity(const IEntityFilter& filter)
	{
		auto& manager = game::EntityManager::instance();
		auto entities = manager.entities(filter);
		return FindNearestEntity(entities);
	}

	game::Entity* FindNearestEntity(std::vector<game::Entity*>& entities)
	{
		if (entities.size() == 0)
			return nullptr;

		auto avatar = game::EntityManager::instance().avatar();
		return *std::min_element(entities.begin(), entities.end(),
			[avatar](game::Entity* a, game::Entity* b) { return avatar->distance(a) < avatar->distance(b); });
	}

	app::AccountDataItem* GetAccountData()
	{
		auto playerModule = GET_SINGLETON(MoleMole_PlayerModule);
		if (playerModule == nullptr || playerModule->fields._accountData_k__BackingField == nullptr)
			return nullptr;

		return playerModule->fields._accountData_k__BackingField;
	}

    bool IsInGame()
    {
		auto loadingManager = GET_SINGLETON(MoleMole_LoadingManager);
        if (loadingManager == nullptr || !app::MoleMole_LoadingManager_IsLoaded(loadingManager, nullptr))
			return false;
		return true;
    }

    void ShowInGameNormalMessage(std::string content, app::Color bgColor, bool showBackground)
    {
		auto uiManager = GET_SINGLETON(MoleMole_UIManager);
		if (uiManager == nullptr)
			return;
		app::MoleMole_UIManager_TryShowNormalMessage(uiManager, string_to_il2cppi(content), bgColor, showBackground, nullptr);
    }

    void ShowInGameInfoMessage(std::string content, bool discardIfDuplicated, app::UIShowPriority__Enum priority)
    {
        auto uiManager = GET_SINGLETON(MoleMole_UIManager);
        if (uiManager == nullptr)
            return;
		app::MoleMole_UIManager_TryShowInfoMessageWithoutBlockInput(uiManager, string_to_il2cppi(content), discardIfDuplicated, priority, nullptr);
    }

    void ShowInGameGeneralDialog(std::string title, std::string content, app::GeneralDialogContext_GeneralDialogType__Enum dialogType, app::UIType__Enum uiType, app::UIShowPriority__Enum mode, bool showNormalCloseBtn, bool isToggleDialog)
    {
		app::MoleMole_UIUtil_TryShowGeneralDialog(string_to_il2cppi(title), string_to_il2cppi(content), nullptr, uiType, nullptr, dialogType, mode, showNormalCloseBtn, isToggleDialog, false, nullptr);
    }

    void ShowInGameSimpleGeneralDialog(std::string title, std::string content)
    {
		app::MoleMole_UIUtil_TryShowSimpleGeneralDialog_1(string_to_il2cppi(title), string_to_il2cppi(content), nullptr, nullptr);
    }

    void ShowInGameNormalMessage(std::wstring content, app::Color bgColor, bool showBackground)
    {
        auto uiManager = GET_SINGLETON(MoleMole_UIManager);
        if (uiManager == nullptr)
            return;
        app::MoleMole_UIManager_TryShowNormalMessage(uiManager, string_to_il2cppi(content), bgColor, showBackground, nullptr);
    }

    void ShowInGameInfoMessage(std::wstring content, bool discardIfDuplicated, app::UIShowPriority__Enum priority)
    {
        auto uiManager = GET_SINGLETON(MoleMole_UIManager);
        if (uiManager == nullptr)
            return;
        app::MoleMole_UIManager_TryShowInfoMessageWithoutBlockInput(uiManager, string_to_il2cppi(content), discardIfDuplicated, priority, nullptr);
    }

    void ShowInGameGeneralDialog(std::wstring title, std::wstring content, app::GeneralDialogContext_GeneralDialogType__Enum dialogType, app::UIType__Enum uiType, app::UIShowPriority__Enum mode, bool showNormalCloseBtn, bool isToggleDialog)
    {
        app::MoleMole_UIUtil_TryShowGeneralDialog(string_to_il2cppi(title), string_to_il2cppi(content), nullptr, uiType, nullptr, dialogType, mode, showNormalCloseBtn, isToggleDialog, false, nullptr);
    }

    void ShowInGameSimpleGeneralDialog(std::wstring title, std::wstring content)
    {
        app::MoleMole_UIUtil_TryShowSimpleGeneralDialog_1(string_to_il2cppi(title), string_to_il2cppi(content), nullptr, nullptr);
    }

}
