#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

#include <cheat/game/IEntityFilter.h>

namespace cheat::feature 
{

	class ItemTeleportBase : public Feature, public game::IEntityFilter
    {
	public:
		config::Field<Hotkey> f_Key;
		config::Field<bool> f_ShowInfo;

		void DrawMain() final;
		
		virtual void DrawItems();
		virtual void DrawFilterOptions() { };

		virtual bool NeedInfoDraw() const;
		virtual void DrawInfo() override;
	
	protected:
		ItemTeleportBase(const std::string& section, const std::string& name);
		
		void DrawEntityInfo(game::Entity* entity);

	private:
		void DrawEntities();
		void DrawNearestEntityInfo();
		
		void OnTeleportKeyPressed();

		std::string section;
		std::string name;
	};
}

