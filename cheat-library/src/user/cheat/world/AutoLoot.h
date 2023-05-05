#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>
#include <cheat-base/thread-safe.h>

namespace cheat::feature 
{

	class AutoLoot : public Feature
    {
	public:
		config::Field<TranslatedHotkey> f_AutoPickup;
		config::Field<TranslatedHotkey> f_AutoDisablePickupWhenAddItemExceedLimit;
		config::Field<TranslatedHotkey> f_AutoTreasure;
		config::Field<TranslatedHotkey> f_UseCustomRange;
		config::Field<TranslatedHotkey> f_UseDelayTimeFluctuation;
		config::Field<TranslatedHotkey> f_PickupFilter;

		config::Field<int> f_DelayTime;
		config::Field<int> f_DelayTimeFluctuation;
		config::Field<float> f_CustomRange;

		config::Field<bool> f_Chest;
		config::Field<bool> f_Leyline;
		config::Field<bool> f_Investigate;
		config::Field<bool> f_QuestInteract;
		config::Field<bool> f_Others;
		
		config::Field<bool> f_PickupFilter_Animals;
		config::Field<bool> f_PickupFilter_DropItems;
		config::Field<bool> f_PickupFilter_Resources;
		config::Field<bool> f_PickupFilter_Oculus;

		static AutoLoot& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

		bool OnCreateButton(app::BaseEntity* entity);
		void OnGameUpdate();
		void OnCheckIsInPosition(bool& result, app::BaseEntity* entity);

		void clear_toBeLootedItems();
	
	private:
		
		SafeQueue<uint32_t> toBeLootedItems;
		SafeValue<int64_t> nextLootTime;
		AutoLoot();
	};
}

