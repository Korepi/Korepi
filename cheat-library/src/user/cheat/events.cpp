#include "pch-il2cpp.h"
#include "events.h"

namespace cheat::events
{
	TEvent<> GameUpdateEvent{};
	TEvent<app::Proto_SceneEntityAppearNotify*> EntityAppearEvent{};
	TEvent<uint32_t> AccountChangedEvent{};
	TEvent<uint32_t, app::MotionInfo*> MoveSyncEvent{};
}