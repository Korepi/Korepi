#pragma once

#include <string>

#include <il2cpp-appdata.h>
#include <imgui.h>

#include <cheat/game/filters.h>
#include <cheat/game/Entity.h>

namespace cheat::feature::esp::render
{
	bool DrawEntity(const std::string& entryName, const std::string& displayName, game::Entity* entity, const ImColor& color, const ImColor& contrastColor);
	void PrepareFrame();
}