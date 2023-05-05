#include <pch.h>

#include "globals.h"

namespace events
{
	TCancelableEvent<short> KeyUpEvent{};
	TCancelableEvent<HWND, UINT, WPARAM, LPARAM> WndProcEvent {};
	TEvent<> RenderEvent {};
}