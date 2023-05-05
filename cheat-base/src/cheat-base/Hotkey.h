#pragma once

#include <string>
#include <unordered_set>

#include <cheat-base/events/event.hpp>

class Hotkey 
{
public:

    Hotkey();
    Hotkey(const Hotkey& other);
    Hotkey(short key);
    Hotkey(std::vector<short> keys);
    ~Hotkey();

	Hotkey& operator=(Hotkey& hotkey) noexcept;
	Hotkey& operator=(Hotkey&& hotkey) noexcept;

	bool operator== (const Hotkey& c2) const;
	bool operator!= (const Hotkey& c2) const;
	bool operator-(const Hotkey& c2);

    bool IsPressed() const;
    bool IsPressed(short keyDown) const;
    bool IsReleased() const;

    bool IsEmpty() const;

    std::vector<short> GetKeys() const;

    operator std::string() const;

    static Hotkey GetPressedHotkey();

    IEvent<>& PressedEvent;
private:
    TEvent<> m_PressedEvent;
    std::unordered_set<short> m_Keys;

    void OnKeyUp(short key, bool& canceled);
};