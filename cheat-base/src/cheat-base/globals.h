#pragma once

#include <Windows.h>

#include <cheat-base/events/event.hpp>

namespace events
{
	extern TCancelableEvent<short> KeyUpEvent;
	extern TCancelableEvent<HWND, UINT, WPARAM, LPARAM> WndProcEvent;
	extern TEvent<> RenderEvent;
}