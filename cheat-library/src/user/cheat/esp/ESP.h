#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/config.h>

#include <cheat/esp/ESPItem.h>
#include <cheat/esp/ESPCustomFilter.h>
#include <cheat/game/IEntityFilter.h>
#include <cheat/game/CacheFilterExecutor.h>
#include <cheat/game/SimpleFilter.h>

#include <cheat-base/cheat/fields/TranslatedHotkey.h>

namespace cheat::feature 
{

	class ESP : public Feature
    {
	public:
		enum class DrawMode
		{
			None,
			Rectangle,
			Box,
			CornerBox
		};

		enum class DrawTracerMode
		{
			None,
			Line,
			OffscreenArrows
		};

		config::Field<TranslatedHotkey> f_Enabled;
    
		config::Field<config::Enum<DrawMode>> f_DrawBoxMode;
		config::Field<float> f_CorneredBoxSize;
		config::Field<config::Enum<DrawTracerMode>> f_DrawTracerMode;
		config::Field<bool> f_Fill;
		config::Field<float> f_FillTransparency;

		config::Field<bool> f_DrawDistance;
		config::Field<bool> f_DrawName;
		config::Field<bool> f_DrawHealth;
		config::Field<bool> f_HideCompleted;

		config::Field<float> f_ArrowRadius;
		config::Field<float> f_TracerSize;
		config::Field<float> f_OutlineThickness;
		config::Field<ImColor> f_OutlineColor;
		config::Field<ImColor> f_CircleColor;
		config::Field<bool> f_ShowArrowIcons;
		config::Field<bool> f_ShowHDIcons;
		config::Field<bool> f_MiddleScreenTracer;

		config::Field<int> f_FontSize;
		config::Field<bool> f_FontOutline;
		config::Field<float> f_FontOutlineSize;

		config::Field<config::Toggle<ImColor>> f_GlobalFontColor;
		ImColor m_FontContrastColor;
		config::Field<config::Toggle<ImColor>> f_GlobalBoxColor;
		config::Field<config::Toggle<ImColor>> f_GlobalRectColor;
		config::Field<config::Toggle<ImColor>> f_GlobalLineColor;

		config::Field<float> f_MinSize;
		config::Field<float> f_Range;

		// custom filters related block
		config::Field<bool> f_ShowCustomFiltersWindow;
		config::Field<nlohmann::json> f_CustomFilterJson;
		std::string m_CustomFilterUiName;
		config::Field<config::Enum<app::EntityType__Enum_1>> f_CustomFilterType;
		std::string m_CustomFilterNameToAdd;
		std::vector<std::string> m_CustomFilterNames;

		std::string m_Search;

		static ESP& GetInstance();

		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;

		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

		void DrawExternal() override;

		void AddCustomFilter(ESPCustomFilter filter, bool convertToJson = false);

		//GadgetInfo GetGadgetInfoFromGadgetId(uint32_t gadgetId);

	private:
		using FilterInfo = std::pair<config::Field<esp::ESPItem>, game::IEntityFilter*>;
		using Filters = std::vector<FilterInfo>;
		using Sections = std::map<std::string, Filters>;

		using CustomFilterInfo = std::pair<config::Field<esp::ESPItem>, ESPCustomFilter*>;
		using CustomFilters = std::vector<CustomFilterInfo>;

		Sections m_Sections;
		game::CacheFilterExecutor m_FilterExecutor;
		std::vector<std::shared_ptr<ESPCustomFilter>> m_CustomFilters;
		CustomFilters m_CustomFilterInfos;
		bool b_DrawCustomFiltersWindow;
		int i_CustomFiltersEditId;
		int i_CustomFilterNameEditId;

		void InstallFilters();
		void InstallCustomFilters();
		void AddFilter(const std::string& section, const std::string& name, game::IEntityFilter* filter);

		void DrawSection(const std::string& section, const Filters& filters);
		void DrawCustomSection();
		void DrawFilterField(const config::Field<esp::ESPItem>& field);
		void DrawCustomFilterNames();
		void DrawCustomFiltersTable();
		void DrawCustomFiltersUi();

		void GetNpcName(std::string& name);
		bool isBuriedChest(game::Entity* entity);
		bool CheckPuzzleFinished(game::Entity* entity);

		void OnKeyUp(short key, bool& cancelled);

		void SaveCustomFilters();

		ESP();
	};
}

