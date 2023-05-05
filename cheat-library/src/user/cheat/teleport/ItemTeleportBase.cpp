#include "pch-il2cpp.h"
#include "ItemTeleportBase.h"

#include <helpers.h>
#include <cheat/teleport/MapTeleport.h>
#include <cheat-base/cheat/CheatManagerBase.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>
#include <cheat/GenshinCM.h>

namespace cheat::feature 
{
    ItemTeleportBase::ItemTeleportBase(const std::string& section, const std::string& name) : Feature(),
		NF(f_Key, section, Hotkey()),
		NF(f_ShowInfo, section, true),
        section(section), name(name)
    {
		f_Key.value().PressedEvent += MY_METHOD_HANDLER(ItemTeleportBase::OnTeleportKeyPressed);
    }

    void ItemTeleportBase::DrawMain()
    {
		auto desc = util::string_format(_TR("When key pressed, will teleport to nearest %s if exists."), name.c_str());
		ConfigWidget(_TR("Teleport to Nearest"), f_Key, true, desc.c_str());

		DrawFilterOptions();

		DrawItems();
    }

	void ItemTeleportBase::DrawItems()
	{
		auto nodeName = util::string_format("%s %s", _TR("list"), name.c_str());
		if (ImGui::TreeNode(nodeName.c_str()))
		{
			DrawEntities();
			ImGui::TreePop();
		}
	}

	bool ItemTeleportBase::NeedInfoDraw() const
{
		return f_ShowInfo;
	}

	void ItemTeleportBase::DrawInfo()
	{
		DrawNearestEntityInfo();
	}

	void ItemTeleportBase::OnTeleportKeyPressed()
	{
		if (GenshinCM::IsMenuShowed())
			return;

		auto entity = game::FindNearestEntity(*this);
		if (entity != nullptr)
		{
			MapTeleport& mapTeleport = MapTeleport::GetInstance();
			mapTeleport.TeleportTo(entity->absolutePosition());
		}
	}

	void ItemTeleportBase::DrawEntityInfo(game::Entity* entity)
	{
		if (entity == nullptr)
		{
			ImGui::Text(Translator::RuntimeTranslate(name).c_str()); ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), _TR("not found"));
			return;
		}
		
		auto& manager = game::EntityManager::instance();
		ImGui::Text(Translator::RuntimeTranslate(name).c_str()); ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.1f, 1.0f), "%.3fm", manager.avatar()->distance(entity));
	}

	void ItemTeleportBase::DrawNearestEntityInfo()
	{
		auto nearestEntity = game::FindNearestEntity(*this);
		DrawEntityInfo(nearestEntity);
	}

	void ItemTeleportBase::DrawEntities()
	{
		auto& manager = game::EntityManager::instance();
		auto entities = manager.entities(*this);
		if (entities.size() == 0)
		{
			ImGui::Text(_TR("Not found."));
			return;
		}

		for (const auto& entity : entities)
		{
			ImGui::Text(_TR("Dist %.03fm"), manager.avatar()->distance(entity));
			ImGui::SameLine();
			auto label = util::string_format(_TR("Teleport ## %p"), entity);
			if (ImGui::Button(label.c_str()))
			{
				MapTeleport& mapTeleport = MapTeleport::GetInstance();
				mapTeleport.TeleportTo(entity->absolutePosition());
			}
		}
	}



}

