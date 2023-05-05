#include <pch-il2cpp.h>
#include "ESPRender.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cheat-base/render/gui-util.h>
#include <cheat-base/render/renderer.h>
#include <helpers.h>
#include <cheat/game/EntityManager.h>

#include <sys/timeb.h>
#include "ESP.h"

namespace cheat::feature::esp::render
{
	static app::Camera* s_Camera = nullptr;
	static ImVec2 s_ResolutionScale = ImVec2(0, 0);
	static ImVec2 s_ScreenResolution = ImVec2(0, 0);
	static ImVec2 s_AvatarPosition = ImVec2(0, 0);

	static void UpdateMainCamera()
	{
		UPDATE_DELAY(1000);

		s_Camera = nullptr;

		auto loadingManager = GET_SINGLETON(MoleMole_LoadingManager);
		if (loadingManager == nullptr || !app::MoleMole_LoadingManager_IsLoaded(loadingManager, nullptr))
			return;

		SAFE_BEGIN();
		auto camera = app::Camera_get_main(nullptr);
		if (camera == nullptr)
			return;

		if (!app::Behaviour_get_isActiveAndEnabled(reinterpret_cast<app::Behaviour*>(camera), nullptr))
			return;
		
		s_Camera = camera;
		SAFE_EEND();
	}

	static void UpdateResolutionScale()
	{
		UPDATE_DELAY(1000);

		SAFE_BEGIN();
		s_ResolutionScale = { 0, 0 };

		if (s_Camera == nullptr)
			return;

		auto pixelWidth = app::Camera_get_pixelWidth(s_Camera, nullptr);
		auto pixelHeight = app::Camera_get_pixelHeight(s_Camera, nullptr);

		if (pixelWidth == 0 || pixelHeight == 0)
			return;

		auto screenWidth = app::Screen_get_width(nullptr);
		auto screenHeight = app::Screen_get_height(nullptr);

		if (screenWidth == 0 || screenHeight == 0)
			return;

		if (screenHeight == pixelHeight && screenWidth == pixelWidth)
			return;

		s_ScreenResolution.x = static_cast<float>(screenWidth);
		s_ScreenResolution.y = static_cast<float>(screenHeight);

		s_ResolutionScale.x = s_ScreenResolution.x / static_cast<float>(pixelWidth);
		s_ResolutionScale.y = s_ScreenResolution.y / static_cast<float>(pixelHeight);
		SAFE_EEND();
	}

	static app::Vector3 WorldToScreenPosScalled(const app::Vector3& relPosition)
	{
		if (s_Camera == nullptr)
			return {};

		auto screenPos = app::Camera_WorldToScreenPoint(s_Camera, relPosition, nullptr);

		if (s_ResolutionScale.x != 0)
		{
			screenPos.x *= s_ResolutionScale.x;
			screenPos.y *= s_ResolutionScale.y;
		}

		screenPos.y = static_cast<float>(app::Screen_get_height(nullptr)) - screenPos.y;
		return screenPos;
	}

	struct Rect
	{
		float xMin, xMax;
		float yMin, yMax;

		bool empty() const
		{
			return xMin == 0 && xMax == 0 && yMin == 0 && yMax == 0;
		}
	};

	struct BoxScreen
	{
		ImVec2 lowerTopLeft;
		ImVec2 lowerTopRight;
		ImVec2 lowerBottomLeft;
		ImVec2 lowerBottomRight;

		ImVec2 upperTopLeft;
		ImVec2 upperTopRight;
		ImVec2 upperBottomLeft;
		ImVec2 upperBottomRight;
	};

#undef min
#undef max

	static ImVec2 FromVec3(const app::Vector3& vec3)
	{
		return { vec3.x, vec3.y };
	}

	static app::Bounds GetEntityMinBounds(game::Entity* entity, float minSize)
	{
		auto entityPosition = entity->relativePosition();
		return { entityPosition, { minSize, minSize, minSize } };
	}

	static app::Bounds GetObjectBounds(game::Entity* entity)
	{
		auto& esp = ESP::GetInstance();
		auto gameObject = entity->gameObject();
		if (gameObject == nullptr)
			return GetEntityMinBounds(entity, esp.f_MinSize);

		SAFE_BEGIN();

		// Sometimes occurs access violation in UnityPlayer.dll
		// Callow: Have no idea what to do with it unless just catch exception
		auto bounds = app::Utils_4_GetBounds(gameObject, nullptr);
		if (bounds.m_Extents.x < esp.f_MinSize &&
			bounds.m_Extents.y < esp.f_MinSize &&
			bounds.m_Extents.z < esp.f_MinSize)
			bounds.m_Extents = { esp.f_MinSize, esp.f_MinSize, esp.f_MinSize };

		auto min = bounds.m_Center - bounds.m_Extents;
		auto max = bounds.m_Center + bounds.m_Extents;

		// When monster or some another object in a far from player - they disappear
		// And for some reason game object extends
		if ((min.x == 0 || min.y == 0 || min.z == 0))
			return GetEntityMinBounds(entity, 1);
		return bounds;
		
		SAFE_ERROR();
		
		return GetEntityMinBounds(entity, esp.f_MinSize);
		
		SAFE_END();
	}

	static std::optional<BoxScreen> GetEntityScreenBox(game::Entity* entity)
	{
		if (s_Camera == nullptr)
			return {};
		app::Bounds bounds = GetObjectBounds(entity); //copy
		app::Vector3 min, max;

		//m0nkrel: Fixing abnormal size of completed elemental monument(Thanks to Taiga)
		if(game::filters::puzzle::ElementalMonument.IsValid(entity))
		{
			if (bounds.m_Extents.x > 0.65f || bounds.m_Extents.y > 0.8f || bounds.m_Extents.z > 0.65f)
				bounds.m_Extents = { 0.65f, 0.8f, 0.65f };
			auto pos = entity->relativePosition();
			bounds.m_Center = { pos.x, pos.y + 1.f, pos.z };
		}
		//m0nkrel :  Fixing abnormal size of gliding avatars. 
		//Hardcoded bounds center and extents because they floating behind avatar
		else if (entity->isAvatar())
		{
			if (bounds.m_Extents.x > 1.f || bounds.m_Extents.y > 1.f || bounds.m_Extents.z > 0.8f)
			{
				bounds.m_Extents = { 1.f, 1.f, 0.8f };
			}
			auto pos = entity->relativePosition();
			bounds.m_Center = { pos.x + 0.12f,pos.y + 1.f, pos.z + 0.12f };
		}
		//m0nkrel : Fixing abnormal size of blocked chests.
		else if (game::filters::chest::SFrozen.IsValid(entity) ||
				 game::filters::chest::SInLock.IsValid(entity) ||
				 game::filters::chest::SBramble.IsValid(entity))
		{
			bounds.m_Extents = bounds.m_Extents/2;
		}
		else if (game::filters::puzzle::ElectroSeelie.IsValid(entity))
		{
			bounds.m_Extents = bounds.m_Extents - app::Vector3{4.5f, 4.5f, 4.5f };
			auto pos = entity->relativePosition();
			bounds.m_Center = { pos.x,pos.y + 1.f, pos.z };
		}

		min = bounds.m_Center - bounds.m_Extents;
		max = bounds.m_Center + bounds.m_Extents;

		BoxScreen box;
		app::Vector3 temp;
#define BOX_FIELD_SET(field, px, py, pz) temp = app::Camera_WorldToScreenPoint(s_Camera, { px, py, pz }, nullptr);\
			if (temp.z < 1) return {};\
			box.##field = FromVec3(temp);

		BOX_FIELD_SET(lowerTopLeft, min.x, min.y, max.z);
		BOX_FIELD_SET(lowerTopRight, max.x, min.y, max.z);
		BOX_FIELD_SET(lowerBottomLeft, min.x, min.y, min.z);
		BOX_FIELD_SET(lowerBottomRight, max.x, min.y, min.z);

		BOX_FIELD_SET(upperTopLeft, min.x, max.y, max.z);
		BOX_FIELD_SET(upperTopRight, max.x, max.y, max.z);
		BOX_FIELD_SET(upperBottomLeft, min.x, max.y, min.z);
		BOX_FIELD_SET(upperBottomRight, max.x, max.y, min.z);

#undef BOX_FIELD_SET

		return box;
	}

	static void ScaleBoxScreen(BoxScreen& boxScreen)
	{
		if (s_ResolutionScale.x != 0)
		{

#define SCALE_FIELD(field) boxScreen.##field##.x *= s_ResolutionScale.x; boxScreen.##field##.y *= s_ResolutionScale.y

			SCALE_FIELD(lowerTopLeft);
			SCALE_FIELD(lowerTopRight);
			SCALE_FIELD(lowerBottomLeft);
			SCALE_FIELD(lowerBottomRight);

			SCALE_FIELD(upperTopLeft);
			SCALE_FIELD(upperTopRight);
			SCALE_FIELD(upperBottomLeft);
			SCALE_FIELD(upperBottomRight);

#undef SCALE_FIELD

		}

		auto screenHeight = static_cast<float>(app::Screen_get_height(nullptr));

#define FIX_Y(field) boxScreen.##field##.y = screenHeight - boxScreen.##field##.y

		FIX_Y(lowerTopLeft);
		FIX_Y(lowerTopRight);
		FIX_Y(lowerBottomLeft);
		FIX_Y(lowerBottomRight);

		FIX_Y(upperTopLeft);
		FIX_Y(upperTopRight);
		FIX_Y(upperBottomLeft);
		FIX_Y(upperBottomRight);

#undef FIX_Y

	}

	static Rect GetEntityScreenRect(const BoxScreen& box, bool scalling = true)
	{
		Rect boxRect{};

		boxRect.xMin = std::min({ box.lowerTopLeft.x, box.lowerTopRight.x, box.lowerBottomLeft.x, box.lowerBottomRight.x,
			box.upperTopLeft.x, box.upperTopRight.x, box.upperBottomRight.x, box.upperBottomLeft.x });
		boxRect.xMax = std::max({ box.lowerTopLeft.x, box.lowerTopRight.x, box.lowerBottomLeft.x, box.lowerBottomRight.x,
			box.upperTopLeft.x, box.upperTopRight.x, box.upperBottomRight.x, box.upperBottomLeft.x });

		boxRect.yMin = std::max({ box.lowerTopLeft.y, box.lowerTopRight.y, box.lowerBottomLeft.y, box.lowerBottomRight.y,
			box.upperTopLeft.y, box.upperTopRight.y, box.upperBottomRight.y, box.upperBottomLeft.y });
		boxRect.yMax = std::min({ box.lowerTopLeft.y, box.lowerTopRight.y, box.lowerBottomLeft.y, box.lowerBottomRight.y,
			box.upperTopLeft.y, box.upperTopRight.y, box.upperBottomRight.y, box.upperBottomLeft.y });

		if (!scalling)
		{
			std::swap(boxRect.yMin, boxRect.yMax);
			return boxRect;
		}

		if (s_ResolutionScale.x != 0)
		{
			boxRect.xMin *= s_ResolutionScale.x;
			boxRect.xMax *= s_ResolutionScale.x;

			boxRect.yMin *= s_ResolutionScale.y;
			boxRect.yMax *= s_ResolutionScale.y;
		}

		float screenHeight = static_cast<float>(app::Screen_get_height(nullptr));
		boxRect.yMin = screenHeight - boxRect.yMin;
		boxRect.yMax = screenHeight - boxRect.yMax;
		return boxRect;
	}

	static void DrawQuadLines(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col)
	{
		auto draw = ImGui::GetBackgroundDrawList();

		draw->AddLine(p1, p2, col);
		draw->AddLine(p2, p3, col);
		draw->AddLine(p3, p4, col);
		draw->AddLine(p4, p1, col);
	}

	static bool HasCenter(const Rect& rect)
	{
		ImVec2 centerPoint = ImVec2(s_ScreenResolution.x / 2, s_ScreenResolution.y / 2);

		return rect.xMin < centerPoint.x&& centerPoint.x < rect.xMax &&
			rect.yMin < centerPoint.y&& centerPoint.y < rect.yMax;
	}

	static Rect DrawRect(game::Entity* entity, const ImColor& color)
	{
		auto box = GetEntityScreenBox(entity);
		if (!box)
			return {};

		auto entityRect = GetEntityScreenRect(*box);
		if (entityRect.empty())
			return {};

		auto& esp = ESP::GetInstance();
		auto draw = ImGui::GetBackgroundDrawList();

		auto pMin = ImVec2(entityRect.xMin, entityRect.yMin);
		auto pMax = ImVec2(entityRect.xMax, entityRect.yMax);
		if (esp.f_Fill)
		{
			ImColor newColor = color;
			newColor.Value.w = 1.0f - esp.f_FillTransparency;
			draw->AddRectFilled(pMin, pMax, newColor);
		}
		draw->AddRect(pMin, pMax, color);

		return entityRect;
	}

	// Callow: This way to drawing is slower than native
	static Rect DrawBox(game::Entity* entity, const ImColor& color)
	{
		auto box = GetEntityScreenBox(entity);
		if (!box)
			return {};

		ScaleBoxScreen(*box);
		auto& esp = ESP::GetInstance();
		auto draw = ImGui::GetBackgroundDrawList();

		if (esp.f_Fill)
		{
			ImColor newColor = color;
			newColor.Value.w = 1.0f - esp.f_FillTransparency;

			float threshold = 2.0f;
#define ADD_FIXED_QUAD(p1, p2, p3, p4, col) {\
                                                ImVec2 p13 { std::abs(p3.x - p1.x), std::abs(p3.y - p1.y) };\
                                                ImVec2 p24 { std::abs(p2.x - p4.x), std::abs(p2.y - p4.y) };\
                                                if ((p13.x > threshold && p13.y > threshold) || \
                                                    (p24.x > threshold && p24.y > threshold) || \
                                                    (p13.x > threshold && p24.y > threshold) || \
                                                    (p24.x > threshold && p13.y > threshold))\
                                                        draw->AddQuadFilled(p1, p2, p3, p4, newColor);\
                                            }

			ADD_FIXED_QUAD(box->lowerBottomLeft, box->lowerTopLeft, box->lowerTopRight, box->lowerBottomRight, newColor);
			ADD_FIXED_QUAD(box->upperBottomLeft, box->upperTopLeft, box->upperTopRight, box->upperBottomRight, newColor);

			ADD_FIXED_QUAD(box->lowerBottomLeft, box->upperBottomLeft, box->upperBottomRight, box->lowerBottomRight, newColor);
			ADD_FIXED_QUAD(box->lowerTopLeft, box->upperTopLeft, box->upperTopRight, box->lowerTopRight, newColor);

			ADD_FIXED_QUAD(box->lowerBottomLeft, box->lowerTopLeft, box->upperTopLeft, box->upperBottomLeft, newColor);
			ADD_FIXED_QUAD(box->lowerBottomRight, box->lowerTopRight, box->upperTopRight, box->upperBottomRight, newColor);

#undef ADD_FIXED_QUAD
		}

		DrawQuadLines(box->lowerBottomLeft, box->lowerTopLeft, box->lowerTopRight, box->lowerBottomRight, color);
		DrawQuadLines(box->upperBottomLeft, box->upperTopLeft, box->upperTopRight, box->upperBottomRight, color);

		draw->AddLine(box->lowerBottomLeft, box->upperBottomLeft, color);
		draw->AddLine(box->lowerTopLeft, box->upperTopLeft, color);
		draw->AddLine(box->lowerTopRight, box->upperTopRight, color);
		draw->AddLine(box->lowerBottomRight, box->upperBottomRight, color);

		auto rect = GetEntityScreenRect(*box, false);
		return rect;
	}

	static void UpdateAvatarPosition()
	{
		SAFE_BEGIN();
		auto& manager = game::EntityManager::instance();
		auto avatar = manager.avatar();
		if (avatar == nullptr)
			return;

		auto avatarPos = WorldToScreenPosScalled(avatar->relativePosition());
		s_AvatarPosition = ImVec2(avatarPos.x, avatarPos.y);
		SAFE_EEND();
	}

	static std::optional<ImVec2> GetEntityScreenPos(game::Entity* entity)
	{
		auto targetPos = WorldToScreenPosScalled(entity->relativePosition());
		if (targetPos.z < 1)
			return {};

		return ImVec2(targetPos.x, targetPos.y);
	}

	static void DrawLine(game::Entity* entity, const ImColor& color)
	{
		auto screenPos = GetEntityScreenPos(entity);
		if (!screenPos)
			return;

		auto draw = ImGui::GetBackgroundDrawList();

		auto& esp = ESP::GetInstance();
		if (esp.f_MiddleScreenTracer)
		{
			ImRect screen_rect = { 0.0f, 0.0f, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y };
			auto screen_center = screen_rect.GetCenter();
			draw->AddLine(screen_center, *screenPos, color);
		}
		else
		{
			draw->AddLine(s_AvatarPosition, *screenPos, color);
		}
			
	}
  
#define PI 3.14159265358979323846f

	static void DrawOffscreenArrows(game::Entity* entity, const ImColor& color, const std::string& entryName)
	{
		ImRect screen_rect = { 0.0f, 0.0f, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y };
		auto entity_pos = WorldToScreenPosScalled(entity->relativePosition());
		if (entity_pos.z == 0 || (entity_pos.z > 0 && screen_rect.Contains({ entity_pos.x, entity_pos.y })))
			return;

		ImVec2 screen_center = screen_rect.GetCenter();
		float angle = atan2(screen_center.y - entity_pos.y, screen_center.x - entity_pos.x);
		angle += entity_pos.z > 0 ? PI : 0.0f;

		float cosA = cosf(angle);
		float sinA = sinf(angle);

		auto& esp = ESP::GetInstance();
		ImVec2 arrow_center{
			screen_center.x + esp.f_ArrowRadius * cosA,
			screen_center.y + esp.f_ArrowRadius * sinA
		};

		// Alpha fade in/out
		float alpha = 1.0f;
		if (entity_pos.z > 0)
		{
			constexpr float nearThreshold = 200.0f * 200.0f;
			ImVec2 screen_outer_diff = {
				entity_pos.x < 0 ? abs(entity_pos.x) : (entity_pos.x > screen_rect.Max.x ? entity_pos.x - screen_rect.Max.x : 0.0f),
				entity_pos.y < 0 ? abs(entity_pos.y) : (entity_pos.y > screen_rect.Max.y ? entity_pos.y - screen_rect.Max.y : 0.0f),
			};
			float distance = powf(screen_outer_diff.x, 2) + powf(screen_outer_diff.y, 2);
			alpha = entity_pos.z < 0 ? 1.0f : (distance / nearThreshold);
		}
		alpha = std::min(alpha, 1.0f);

		// Copy
		auto arrowColor = color;
		auto lineColor = esp.f_OutlineColor.value();
		auto circleColor = esp.f_CircleColor.value();

		// Apply transparency
		arrowColor.Value.w = alpha;
		lineColor.Value.w = alpha;
		circleColor.Value.w = alpha;

		// Triangle dimensions
		const float height = 22.0f;
		const float width = 8.6f;

		// Tangent circle center and radius
		const float magnitude = sqrtf(powf(height, 2) + powf(width, 2));
		const ImVec2 normal = ImVec2(height, width) / magnitude;
		const float hypotenuse = width / normal.x;
		const float base = hypotenuse * normal.y;

		std::array<ImVec2, 4> points {
		// Triangle points
			ImVec2(-height, -width),
			ImVec2(0.0f, 0.0f),
			ImVec2(-height, width),
		// Circle center
			ImVec2(-height - base, 0.0f)
		};

		// Adjust points
		for (auto& point : points)
		{
			float x = point.x * esp.f_TracerSize;
			float y = point.y * esp.f_TracerSize;
			point.x = arrow_center.x + x * cosA - y * sinA;
			point.y = arrow_center.y + x * sinA + y * cosA;
		}

		float radius = hypotenuse * esp.f_TracerSize;
		auto draw = ImGui::GetBackgroundDrawList();

		draw->AddCircleFilled(points[3], radius + esp.f_OutlineThickness, lineColor); // Circle outline
		draw->AddTriangleFilled(points[0], points[1], points[2], arrowColor);
		draw->AddTriangle(points[0], points[1], points[2], lineColor, esp.f_OutlineThickness);
		draw->AddCircleFilled(points[3], radius, arrowColor); // Circle BG

		if (esp.f_ShowArrowIcons && !entryName.empty())
		{
			auto imageInfo = ImageLoader::GetImage(util::Unsplit(esp.f_ShowHDIcons ? "HD" + entryName : entryName));
			auto textureID = imageInfo ? imageInfo->textureID : nullptr;

			if (textureID)
			{
				radius *= 0.9f;
				ImVec2 imageStartPos = points[3] - radius;
				ImVec2 imageEndPos = points[3] + radius;

				draw->AddCircleFilled(points[3], radius, circleColor); // Circle icon fill
				draw->AddImageRounded(textureID, imageStartPos + 2.0f, imageEndPos - 2.0f,
					ImVec2(0, 0), ImVec2(1, 1), ImColor(1.0f, 1.0f, 1.0f, arrowColor.Value.w), radius);
			}
		}
	}

	static void DrawName(const Rect& boxRect, game::Entity* entity, const std::string& name, const ImColor& color, const ImColor& contrastColor)
	{
		auto& esp = ESP::GetInstance();
		auto& manager = game::EntityManager::instance();
		const float fontSize = static_cast<float>(esp.f_FontSize);

		std::string text;
		if (esp.f_DrawName && esp.f_DrawDistance)
			text = fmt::format("{} | {:.1f}m", name, manager.avatar()->distance(entity));
		else if (esp.f_DrawDistance)
			text = fmt::format("{:.1f}m", manager.avatar()->distance(entity));
		else
			text = name;
	
		ImVec2 namePosition;
		if (!boxRect.empty())
			namePosition = { boxRect.xMin, boxRect.yMin - fontSize };
		else
		{
			auto screenPos = GetEntityScreenPos(entity);
			if (!screenPos)
				return;
			namePosition = *screenPos;

			// Might need to be aware of performance hit but there shouldn't be any.
			ImGuiContext& g = *GImGui;
			ImFont* font = g.Font;
			auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, FLT_MAX, text.c_str());
			namePosition.x -= (textSize.x / 2.0f);
			namePosition.y -= fontSize;
		}

		auto draw = ImGui::GetBackgroundDrawList();
		static auto fontToken = renderer::CreateFontToken(renderer::GetDefaultFontName(), fontSize);
		
		if (renderer::GetTokenFontName(fontToken) != renderer::GetDefaultFontName())
			renderer::SetTokenFont(fontToken, renderer::GetDefaultFontName());

		if (renderer::GetTokenFontSize(fontToken) != fontSize)
			renderer::SetTokenFontSize(fontToken, fontSize);

		auto font = renderer::GetTokenFont(fontToken);
		// Outline
		if (esp.f_FontOutline)
			DrawTextWithOutline(draw, font, fontSize, namePosition, text.c_str(), color, esp.f_FontOutlineSize, OutlineSide::All, contrastColor);
		else
			draw->AddText(font, fontSize, namePosition, color, text.c_str());
	}

	static void DrawHealth(const Rect& boxRect, game::Entity* entity, const ImColor& color, const ImColor& contrastColor)
	{
		if (!entity->isMonster() && !entity->isAvatar())
			return;

		auto& esp = ESP::GetInstance();
		const float fontSize = static_cast<float>(esp.f_FontSize);
		auto baseCombat = entity->combat();
		if (baseCombat == nullptr)
			return;

		auto HP = app::MoleMole_SafeFloat_get_Value(baseCombat->fields._combatProperty_k__BackingField->fields.HP, nullptr);
		HP = trunc(round(HP));

		if (HP == 0)
			return;
		
		std::string text;
		text = fmt::format("HP: {}", HP);

		ImVec2 healthPosition;
		if (!boxRect.empty()) {
			healthPosition = { boxRect.xMin, boxRect.yMin - fontSize };
		}
		else
		{
			auto screenPos = GetEntityScreenPos(entity);
			if (!screenPos)
				return;
			healthPosition = *screenPos;

			// Might need to be aware of performance hit but there shouldn't be any.
			ImGuiContext& g = *GImGui;
			ImFont* font = g.Font;
			auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, FLT_MAX, text.c_str());
			healthPosition.x -= (textSize.x / 2.0f);
			healthPosition.y -= fontSize;
		}

		healthPosition.y += fontSize;
		auto draw = ImGui::GetBackgroundDrawList();
		static auto fontToken = renderer::CreateFontToken(renderer::GetDefaultFontName(), fontSize);

		if (renderer::GetTokenFontName(fontToken) != renderer::GetDefaultFontName())
			renderer::SetTokenFont(fontToken, renderer::GetDefaultFontName());

		if (renderer::GetTokenFontSize(fontToken) != fontSize)
			renderer::SetTokenFontSize(fontToken, fontSize);

		auto font = renderer::GetTokenFont(fontToken);
		// Outline
		if (esp.f_FontOutline)
			DrawTextWithOutline(draw, font, fontSize, healthPosition, text.c_str(), color, esp.f_FontOutlineSize, OutlineSide::All, contrastColor);
		else
			draw->AddText(font, fontSize, healthPosition, color, text.c_str());
	}

	static Rect DrawCorneredBox(game::Entity* entity, const ImColor& color)
	{
		auto box = GetEntityScreenBox(entity);
		if (!box)
			return {};

		auto entityRect = GetEntityScreenRect(*box);
		if (entityRect.empty())
			return {};

		auto& esp = ESP::GetInstance();
		auto draw = ImGui::GetBackgroundDrawList();

		ImVec2 leftTop = { entityRect.xMin, entityRect.yMax };
		ImVec2 leftBot = { entityRect.xMin, entityRect.yMin };
		ImVec2 rightTop = { entityRect.xMax, entityRect.yMax };
		ImVec2 rightBot = { entityRect.xMax, entityRect.yMin };

		//left top
		draw->AddLine(leftTop, leftTop + ImVec2((rightTop.x - leftTop.x) / esp.f_CorneredBoxSize, 0), color);
		draw->AddLine(leftTop, leftTop + ImVec2(0, -(leftTop.y - leftBot.y) / esp.f_CorneredBoxSize), color);

		//left bot
		draw->AddLine(leftBot, leftBot + ImVec2((rightBot.x - leftBot.x) / esp.f_CorneredBoxSize, 0), color);
		draw->AddLine(leftBot, leftBot + ImVec2(0, (rightTop.y - rightBot.y) / esp.f_CorneredBoxSize), color);

		//right top
		draw->AddLine(rightTop, rightTop + ImVec2((leftTop.x - rightTop.x) / esp.f_CorneredBoxSize, 0), color);
		draw->AddLine(rightTop, rightTop + ImVec2(0, -(rightTop.y - rightBot.y) / esp.f_CorneredBoxSize), color);

		//right bot
		draw->AddLine(rightBot, rightBot + ImVec2(0, (rightTop.y - rightBot.y) / esp.f_CorneredBoxSize), color);
		draw->AddLine(rightBot, rightBot + ImVec2((leftBot.x - rightBot.x) / esp.f_CorneredBoxSize, 0), color);

		return entityRect;
	}

	bool DrawEntity(const std::string& entryName, const std::string& displayName, game::Entity* entity, const ImColor& color, const ImColor& contrastColor)
	{
		SAFE_BEGIN();
		auto& esp = ESP::GetInstance();

		Rect rect;
		switch (esp.f_DrawBoxMode.value())
		{
		case ESP::DrawMode::Box:
			rect = DrawBox(entity, esp.f_GlobalBoxColor ? esp.f_GlobalBoxColor : color);
			break;
		case ESP::DrawMode::Rectangle:
			rect = DrawRect(entity, esp.f_GlobalRectColor ? esp.f_GlobalRectColor : color);
			break;
		case ESP::DrawMode::CornerBox:
			rect = DrawCorneredBox(entity, esp.f_GlobalBoxColor ? esp.f_GlobalBoxColor : color);
		default:
			rect = {};
			break;
		}

		switch (esp.f_DrawTracerMode.value())
		{
		case ESP::DrawTracerMode::Line:
			DrawLine(entity, esp.f_GlobalLineColor ? esp.f_GlobalLineColor : color);
			break;
		case ESP::DrawTracerMode::OffscreenArrows:
			DrawOffscreenArrows(entity, esp.f_GlobalLineColor ? esp.f_GlobalLineColor : color, entryName);
			break;
		default:
			break;
		}

		if (esp.f_DrawName || esp.f_DrawDistance)
			DrawName(rect, entity, displayName, esp.f_GlobalFontColor ? esp.f_GlobalFontColor : color,
				esp.m_FontContrastColor ? esp.m_FontContrastColor : contrastColor);
		if (esp.f_DrawHealth)
			DrawHealth(rect, entity, esp.f_GlobalFontColor ? esp.f_GlobalFontColor : color,
				esp.m_FontContrastColor ? esp.m_FontContrastColor : contrastColor);

		return HasCenter(rect);
		SAFE_ERROR();
		return false;
		SAFE_END();
	}

	void PrepareFrame()
	{
		UpdateMainCamera();
		UpdateResolutionScale();

		auto& esp = ESP::GetInstance();
		if (esp.f_DrawTracerMode.value() != ESP::DrawTracerMode::None)
			UpdateAvatarPosition();
	}
}
