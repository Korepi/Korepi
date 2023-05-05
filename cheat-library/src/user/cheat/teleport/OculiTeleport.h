#pragma once
#include "ItemTeleportBase.h"

namespace cheat::feature 
{

	class OculiTeleport : public ItemTeleportBase
    {
	public:
		static OculiTeleport& GetInstance();
		const FeatureGUIInfo& GetGUIInfo() const override;

		bool IsValid(game::Entity* entity) const override;

	private:
		OculiTeleport();
	};
}

