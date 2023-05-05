#pragma once

#include <il2cpp-appdata.h>
#include <cheat-base/events/event.hpp>

namespace cheat::events
{
	extern TEvent<> GameUpdateEvent;
	extern TEvent<app::Proto_SceneEntityAppearNotify*> EntityAppearEvent;
	extern TEvent<uint32_t> AccountChangedEvent;
	extern TEvent<uint32_t, app::MotionInfo*> MoveSyncEvent;
}