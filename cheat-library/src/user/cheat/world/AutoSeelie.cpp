#include "pch-il2cpp.h"
#include "AutoSeelie.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>
#include <cheat/game/filters.h>

namespace cheat::feature
{
	AutoSeelie::AutoSeelie() : Feature(),
		NFP(f_Enabled, "Auto Seelie", "Auto Seelie", false),
		NF(f_ElectroSeelie, "Auto Seelie", false),
		nextTime(0)
	{
		events::GameUpdateEvent += MY_METHOD_HANDLER(AutoSeelie::OnGameUpdate);
	}
	const FeatureGUIInfo& AutoSeelie::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Auto Seelie", "World");
		return info;
	}

	void AutoSeelie::DrawMain()
	{
		ConfigWidget(_TR("Auto Seelie"), f_Enabled, _TR("Auto follow seelie to its home"));

		if (f_Enabled)
		{
			ImGui::Indent();
			ConfigWidget(_TR("Auto Electro seelie"), f_ElectroSeelie, _TR("Since you don't need to manually start electroseelie, \n"
				"they start moving automatically with this option within 100m radius."));
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 165, 0, 255), _TR("Read the note!"));
			ImGui::Unindent();
		}

	}

	bool AutoSeelie::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}

	void AutoSeelie::DrawStatus()
	{
		ImGui::Text("%s %s", _TR("Auto Seelie"), f_ElectroSeelie ? "+ Electro" : "");
	}

	AutoSeelie& AutoSeelie::GetInstance()
	{
		static AutoSeelie instance;
		return instance;
	}

	bool AutoSeelie::IsEntityForVac(game::Entity* entity)
	{
		auto& manager = game::EntityManager::instance();
		auto distance = manager.avatar()->distance(entity);
		float radius = 100.0f;

		if (game::filters::puzzle::Seelie.IsValid(entity) || 
			game::filters::puzzle::WarmingSeelie.IsValid(entity) || 
			game::filters::puzzle::LuminousSeelie.IsValid(entity) ||
			game::filters::puzzle::ElectroSeelie.IsValid(entity))
		{
			if (entity->name().find("ElectricSeelie") != std::string::npos)
			{

				if (f_ElectroSeelie)
				{
					auto EntityGameObject = app::MoleMole_BaseEntity_get_rootGameObject(entity->raw(), nullptr);
					if (EntityGameObject == nullptr)
						return false;
					auto Transform = app::GameObject_GetComponentByName(EntityGameObject, string_to_il2cppi("Transform"), nullptr);
					auto child = app::Transform_GetChild(reinterpret_cast<app::Transform*>(Transform), 1, nullptr);
					auto pre_status = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(child), nullptr);
					auto status = app::GameObject_get_active(reinterpret_cast<app::GameObject*>(pre_status), nullptr);

					if (status)
					{
						return false;
					}
					return distance <= radius;
				}
				return false;
			}
			return distance <= radius;
		}
		return false;
	}

	void AutoSeelie::OnGameUpdate()
	{
		if (!f_Enabled->enabled())
			return;

		auto currentTime = util::GetCurrentTimeMillisec();
		if (currentTime < nextTime)
			return;

		auto& manager = game::EntityManager::instance();
		auto avatarEntity = manager.avatar();
		for (const auto& entity : manager.entities())
		{
			if (!IsEntityForVac(entity))
				continue;

			entity->setRelativePosition(avatarEntity->relativePosition());
		}
		nextTime = currentTime + 1000;
	}

}