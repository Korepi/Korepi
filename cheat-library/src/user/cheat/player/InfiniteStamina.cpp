#include "pch-il2cpp.h"
#include "InfiniteStamina.h"
#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
	InfiniteStamina::InfiniteStamina() : Feature(),
										 NFP(f_Enabled, "InfiniteStamina", "Infinite Stamina", false),
										 NF(f_PacketReplacement, "InfiniteStamina", false)
	{
		HookManager::install(app::MoleMole_DataItem_HandleNormalProp, DataItem_HandleNormalProp_Hook);
		HookManager::install(app::VCHumanoidMove_MNKKEGMDFFO, VCHumanoidMove_MNKKEGMDFFO_Hook);
		events::MoveSyncEvent += MY_METHOD_HANDLER(InfiniteStamina::OnMoveSync);
	}

	const FeatureGUIInfo &InfiniteStamina::GetGUIInfo() const
	{
		TRANSLATED_GROUP_INFO("Infinite Stamina", "Player");
		return info;
	}

	void InfiniteStamina::DrawMain()
	{
		ConfigWidget(_TR("Enabled"), f_Enabled, _TR("Enables infinite stamina option."));

		ConfigWidget(_TR("Move Sync Packet Replacement"), f_PacketReplacement,
					 _TR("This mode prevents sending server packets with stamina cost actions,\n"
						 "e.g. swim, climb, sprint, etc.\n"
						 "NOTE: This is may be more safe than the standard method. More testing is needed."));
	}

	bool InfiniteStamina::NeedStatusDraw() const
	{
		return f_Enabled->enabled();
	}

	void InfiniteStamina::DrawStatus()
	{
		ImGui::Text("%s [%s]", _TR("Inf. Stamina"), f_PacketReplacement ? _TR("Packet") : _TR("Normal"));
	}

	InfiniteStamina &InfiniteStamina::GetInstance()
	{
		static InfiniteStamina instance;
		return instance;
	}

	// Infinite stamina offline mode. Blocks changes for stamina property.
	// Note. Changes received from the server (not sure about this for current time),
	//       that means that server know our stamina, and changes it in client can be detected.
	// Not working for water because server sending drown action when your stamina down to zero. (Also guess for now)
	bool InfiniteStamina::OnPropertySet(app::PropType__Enum propType)
	{
		using PT = app::PropType__Enum;
		static bool override_cheat = true;

		if (propType == PT::PROP_CUR_TEMPORARY_STAMINA)
			override_cheat = true;

		const bool result = !f_Enabled->enabled() || f_PacketReplacement || override_cheat ||
							(propType != PT::PROP_MAX_STAMINA &&
							 propType != PT::PROP_CUR_PERSIST_STAMINA &&
							 propType != PT::PROP_CUR_TEMPORARY_STAMINA);

		if (propType == PT::PROP_MAX_STAMINA)
			override_cheat = false;

		return result;
	}

	// Infinite stamina packet mode.
	// Note. Blocking packets with movement information, to prevent ability server to know stamina info.
	//       But server may see incorrect movements. What mode safer don't tested.
	void InfiniteStamina::OnMoveSync(uint32_t entityId, app::MotionInfo *syncInfo)
	{
		static bool afterDash = false;

		auto &manager = game::EntityManager::instance();
		auto entity = manager.entity(entityId);
		if (entity->type() == app::EntityType__Enum_1::Vehicle || entity->isAvatar())
		{
			// LOG_DEBUG("Movement packet: %s", magic_enum::enum_name(syncInfo->fields.motionState).data());
			if (f_Enabled->enabled() && f_PacketReplacement)
			{
				auto state = syncInfo->fields.motionState;
				switch (state)
				{
				case app::MotionState__Enum::MotionDash:
				case app::MotionState__Enum::MotionClimb:
				case app::MotionState__Enum::MotionClimbJump:
				case app::MotionState__Enum::MotionStandbyToClimb:
				case app::MotionState__Enum::MotionSwimDash:
				case app::MotionState__Enum::MotionSwimIdle:
				case app::MotionState__Enum::MotionSwimMove:
				case app::MotionState__Enum::MotionSwimJump:
				case app::MotionState__Enum::MotionFly:
				case app::MotionState__Enum::MotionFight:
				case app::MotionState__Enum::MotionDashBeforeShake:
				case app::MotionState__Enum::MotionDangerDash:
					syncInfo->fields.motionState = app::MotionState__Enum::MotionRun;
					break;
				case app::MotionState__Enum::MotionJump:
					if (afterDash)
						syncInfo->fields.motionState = app::MotionState__Enum::MotionRun;
					break;
				case app::MotionState__Enum::MotionSkiffDash:
				case app::MotionState__Enum::MotionSkiffPoweredDash:
					syncInfo->fields.motionState = app::MotionState__Enum::MotionSkiffNormal;
					break;
				}
				if (state != app::MotionState__Enum::MotionJump && state != app::MotionState__Enum::MotionFallOnGround)
					afterDash = state == app::MotionState__Enum::MotionDash;
			}
		}
	}

	void InfiniteStamina::DataItem_HandleNormalProp_Hook(app::DataItem *__this, uint32_t type, int64_t value, app::DataPropOp__Enum state, MethodInfo *method)
	{
		auto &infiniteStamina = GetInstance();

		auto propType = static_cast<app::PropType__Enum>(type);
		bool isValid = infiniteStamina.OnPropertySet(propType);
		if (isValid)
			CALL_ORIGIN(DataItem_HandleNormalProp_Hook, __this, type, value, state, method);
	}

	// Infinite Stamina for Wanderer alone.
	void InfiniteStamina::VCHumanoidMove_MNKKEGMDFFO_Hook(app::VCHumanoidMove *__this, float JJJEOEHLNGP, MethodInfo *method)
	{
		auto &infiniteStamina = GetInstance();

		if (infiniteStamina.f_Enabled || infiniteStamina.f_PacketReplacement)
			JJJEOEHLNGP = 0.f;

		CALL_ORIGIN(VCHumanoidMove_MNKKEGMDFFO_Hook, __this, JJJEOEHLNGP, method);
	}
}