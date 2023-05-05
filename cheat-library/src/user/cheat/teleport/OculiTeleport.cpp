#include "pch-il2cpp.h"
#include "OculiTeleport.h"

#include <helpers.h>
#include <cheat/game/filters.h>

namespace cheat::feature 
{
    OculiTeleport::OculiTeleport() : ItemTeleportBase("OculiTeleport", "Oculi")
    { }

	const FeatureGUIInfo& OculiTeleport::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("0culi Teleport", "Teleport");
		return info;
	}

    OculiTeleport& OculiTeleport::GetInstance()
	{
		static OculiTeleport instance;
		return instance;
	}

	bool OculiTeleport::IsValid(game::Entity* entity) const
	{
		return game::filters::combined::Oculies.IsValid(entity);
	}

}

