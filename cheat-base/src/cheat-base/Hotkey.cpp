#include <pch.h>
#include "Hotkey.h"

#define IM_VK_KEYPAD_ENTER      (VK_RETURN + 256)

static ImGuiKey LegacyToInput(short key)
{
	switch (key)
	{
	case VK_TAB: return ImGuiKey_Tab;
	case VK_LEFT: return ImGuiKey_LeftArrow;
	case VK_RIGHT: return ImGuiKey_RightArrow;
	case VK_UP: return ImGuiKey_UpArrow;
	case VK_DOWN: return ImGuiKey_DownArrow;
	case VK_PRIOR: return ImGuiKey_PageUp;
	case VK_NEXT: return ImGuiKey_PageDown;
	case VK_HOME: return ImGuiKey_Home;
	case VK_END: return ImGuiKey_End;
	case VK_INSERT: return ImGuiKey_Insert;
	case VK_DELETE: return ImGuiKey_Delete;
	case VK_BACK: return ImGuiKey_Backspace;
	case VK_SPACE: return ImGuiKey_Space;
	case VK_RETURN: return ImGuiKey_Enter;
	case VK_ESCAPE: return ImGuiKey_Escape;
	case VK_OEM_7: return ImGuiKey_Apostrophe;
	case VK_OEM_COMMA: return ImGuiKey_Comma;
	case VK_OEM_MINUS: return ImGuiKey_Minus;
	case VK_OEM_PERIOD: return ImGuiKey_Period;
	case VK_OEM_2: return ImGuiKey_Slash;
	case VK_OEM_1: return ImGuiKey_Semicolon;
	case VK_OEM_PLUS: return ImGuiKey_Equal;
	case VK_OEM_4: return ImGuiKey_LeftBracket;
	case VK_OEM_5: return ImGuiKey_Backslash;
	case VK_OEM_6: return ImGuiKey_RightBracket;
	case VK_OEM_3: return ImGuiKey_GraveAccent;
	case VK_CAPITAL: return ImGuiKey_CapsLock;
	case VK_SCROLL: return ImGuiKey_ScrollLock;
	case VK_NUMLOCK: return ImGuiKey_NumLock;
	case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
	case VK_PAUSE: return ImGuiKey_Pause;
	case VK_NUMPAD0: return ImGuiKey_Keypad0;
	case VK_NUMPAD1: return ImGuiKey_Keypad1;
	case VK_NUMPAD2: return ImGuiKey_Keypad2;
	case VK_NUMPAD3: return ImGuiKey_Keypad3;
	case VK_NUMPAD4: return ImGuiKey_Keypad4;
	case VK_NUMPAD5: return ImGuiKey_Keypad5;
	case VK_NUMPAD6: return ImGuiKey_Keypad6;
	case VK_NUMPAD7: return ImGuiKey_Keypad7;
	case VK_NUMPAD8: return ImGuiKey_Keypad8;
	case VK_NUMPAD9: return ImGuiKey_Keypad9;
	case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
	case VK_DIVIDE: return ImGuiKey_KeypadDivide;
	case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
	case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
	case VK_ADD: return ImGuiKey_KeypadAdd;
	case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
	case VK_LSHIFT: return ImGuiKey_LeftShift;
	case VK_LCONTROL: return ImGuiKey_LeftCtrl;
	case VK_LMENU: return ImGuiKey_LeftAlt;
	case VK_LWIN: return ImGuiKey_LeftSuper;
	case VK_RSHIFT: return ImGuiKey_RightShift;
	case VK_RCONTROL: return ImGuiKey_RightCtrl;
	case VK_RMENU: return ImGuiKey_RightAlt;
	case VK_RWIN: return ImGuiKey_RightSuper;
	case VK_APPS: return ImGuiKey_Menu;
	case '0': return ImGuiKey_0;
	case '1': return ImGuiKey_1;
	case '2': return ImGuiKey_2;
	case '3': return ImGuiKey_3;
	case '4': return ImGuiKey_4;
	case '5': return ImGuiKey_5;
	case '6': return ImGuiKey_6;
	case '7': return ImGuiKey_7;
	case '8': return ImGuiKey_8;
	case '9': return ImGuiKey_9;
	case 'A': return ImGuiKey_A;
	case 'B': return ImGuiKey_B;
	case 'C': return ImGuiKey_C;
	case 'D': return ImGuiKey_D;
	case 'E': return ImGuiKey_E;
	case 'F': return ImGuiKey_F;
	case 'G': return ImGuiKey_G;
	case 'H': return ImGuiKey_H;
	case 'I': return ImGuiKey_I;
	case 'J': return ImGuiKey_J;
	case 'K': return ImGuiKey_K;
	case 'L': return ImGuiKey_L;
	case 'M': return ImGuiKey_M;
	case 'N': return ImGuiKey_N;
	case 'O': return ImGuiKey_O;
	case 'P': return ImGuiKey_P;
	case 'Q': return ImGuiKey_Q;
	case 'R': return ImGuiKey_R;
	case 'S': return ImGuiKey_S;
	case 'T': return ImGuiKey_T;
	case 'U': return ImGuiKey_U;
	case 'V': return ImGuiKey_V;
	case 'W': return ImGuiKey_W;
	case 'X': return ImGuiKey_X;
	case 'Y': return ImGuiKey_Y;
	case 'Z': return ImGuiKey_Z;
	case VK_F1: return ImGuiKey_F1;
	case VK_F2: return ImGuiKey_F2;
	case VK_F3: return ImGuiKey_F3;
	case VK_F4: return ImGuiKey_F4;
	case VK_F5: return ImGuiKey_F5;
	case VK_F6: return ImGuiKey_F6;
	case VK_F7: return ImGuiKey_F7;
	case VK_F8: return ImGuiKey_F8;
	case VK_F9: return ImGuiKey_F9;
	case VK_F10: return ImGuiKey_F10;
	case VK_F11: return ImGuiKey_F11;
	case VK_F12: return ImGuiKey_F12;
	case VK_LBUTTON: return ImGuiMouseButton_Left;
	case VK_RBUTTON: return ImGuiMouseButton_Right;
	case VK_MBUTTON: return ImGuiMouseButton_Middle;
	case VK_XBUTTON1: return 3;
	case VK_XBUTTON2: return 4;
	default: return ImGuiKey_None;
	}
}

static short InputToLegacy(ImGuiKey inputkey)
{
	auto& io = ImGui::GetIO();
	if (inputkey > 4)
		return io.KeyMap[inputkey];

	switch (inputkey)
	{
	case ImGuiMouseButton_Left:
		return VK_LBUTTON;
	case ImGuiMouseButton_Right:
		return VK_RBUTTON;
	case ImGuiMouseButton_Middle:
		return VK_MBUTTON;
	case 3:
		return VK_XBUTTON1;
	case 4:
		return VK_XBUTTON2;
	}

	LOG_CRIT("Failed to find legacy input");
	return -1;
}

static bool IsKeyDown(ImGuiKey key)
{
	if (key > 6)
		return ImGui::IsKeyDown(key);
	
	switch (key)
	{
	case 1:
	case 2:
		return ImGui::IsMouseDown(key - 1);
	case 4:
	case 5:
	case 6:
		return ImGui::IsMouseDown(key - 2);
	}
	return false;
}

static bool IsKeyReleased(ImGuiKey key)
{
	if (key > 6)
		return ImGui::IsKeyReleased(key);

	switch (key)
	{
	case 1:
	case 2:
		return ImGui::IsMouseReleased(key - 1);
	case 4:
	case 5:
	case 6:
		return ImGui::IsMouseReleased(key - 2);
	}
	return false;
}

void FixModKey(short& legacyKey)
{
	// Can cause incorrect input when both keys pressed, need to fix!
	switch (legacyKey)
	{

	case VK_CONTROL:
	{
		if (IsKeyDown(ImGuiKey_LeftCtrl))
			legacyKey = VK_LCONTROL;
		else if (IsKeyDown(ImGuiKey_RightCtrl))
			legacyKey = VK_RCONTROL;

		return;
	}
	case VK_SHIFT:
	{
		if (IsKeyDown(ImGuiKey_LeftShift))
			legacyKey = VK_LSHIFT;
		else if (IsKeyDown(ImGuiKey_RightShift))
			legacyKey = VK_RSHIFT;

		return;
	}

	}
}

Hotkey::Hotkey() : PressedEvent(m_PressedEvent), m_PressedEvent()
{
	events::KeyUpEvent += MY_METHOD_HANDLER(Hotkey::OnKeyUp);
}

Hotkey::Hotkey(std::vector<short> legacyKeys) : Hotkey()
{
    for (short legacyKey : legacyKeys)
    {
        this->m_Keys.insert(legacyKey);
    }
}

Hotkey::Hotkey(short key) : Hotkey()
{
    this->m_Keys.insert(key);
}

Hotkey::Hotkey(const Hotkey& other) : Hotkey()
{
	m_Keys = {other.m_Keys};
}

Hotkey::~Hotkey()
{
	events::KeyUpEvent -= MY_METHOD_HANDLER(Hotkey::OnKeyUp);
}

Hotkey& Hotkey::operator=(Hotkey&& hotkey) noexcept
{
	m_Keys = std::move(hotkey.m_Keys);
	return *this;
}

Hotkey& Hotkey::operator=(Hotkey& hotkey) noexcept
{
	m_Keys = hotkey.m_Keys;
	return *this;
}

bool Hotkey::operator-(const Hotkey& c2)
{
	for (short key : m_Keys)
	{
		if (c2.m_Keys.count(key) == 0)
			return true;
	}
	return false;
}

bool Hotkey::operator!=(const Hotkey& c2) const
{
	return !(*this == c2);
}

bool Hotkey::operator==(const Hotkey& c2) const
{
	return m_Keys == c2.m_Keys;
}

std::string GetKeyName(short key)
{
    if (key > 5)
        return ImGui::GetKeyName(key);
    
    switch (key)
    {
    case ImGuiMouseButton_Left:
        return "LMB";
    case ImGuiMouseButton_Right:
        return "RMB";
    case ImGuiMouseButton_Middle:
        return "MMB";
    case 3:
        return "Mouse X1";
    case 4:
        return "Mouse X2";
    }

    return "Unknown";
}

Hotkey::operator std::string() const 
{
    if (IsEmpty())
        return "None";

    std::stringstream hotkeyNameStream;

    for (auto it = m_Keys.begin(); it != m_Keys.end(); it++)
    {
        if (it != m_Keys.begin())
            hotkeyNameStream << " + ";

        hotkeyNameStream << GetKeyName(LegacyToInput(*it));
    }
    return hotkeyNameStream.str();
}

bool Hotkey::IsPressed() const
{
	for (short key : m_Keys)
	{
		if (!IsKeyDown(key))
			return false;
	}

	return true;
}

bool Hotkey::IsPressed(short legacyKey) const
{
	FixModKey(legacyKey);

    if (m_Keys.count(legacyKey) == 0)
        return false;

    std::unordered_set<short> keysClone = m_Keys;
    keysClone.erase(legacyKey);

    for (short key : keysClone)
    {
		bool result = IsKeyDown(key);
        if (!result)
            return false;
    }

    return true;
}

bool Hotkey::IsReleased() const
{
	bool released = false;
	for (short key : m_Keys)
	{
		if (IsKeyReleased(key))
		{
			released = true;
			continue;
		}

		if (!IsKeyDown(key))
			return false;
	}

	return released;
}

bool Hotkey::IsEmpty() const
{
    return m_Keys.size() == 0;
}

std::vector<short> Hotkey::GetKeys() const
{
    return std::vector<short>(m_Keys.begin(), m_Keys.end());
}

Hotkey Hotkey::GetPressedHotkey()
{
    Hotkey hotkey{};

    auto& io = ImGui::GetIO();

    for (ImGuiKey i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END - 4; i++)
    {
        bool isKeyDown = io.KeysDown[i];
        if (isKeyDown)
            hotkey.m_Keys.insert(InputToLegacy(i));
    }

    for (ImGuiKey i = 0; i < ImGuiMouseButton_COUNT; i++)
    {
        bool isMouseButtonDown = io.MouseDown[i];
        if (isMouseButtonDown)
            hotkey.m_Keys.insert(InputToLegacy(i));
    }
    return hotkey;
}

void Hotkey::OnKeyUp(short key, bool& canceled)
{
	if (IsPressed(key))
		m_PressedEvent();
}
