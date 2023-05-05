#pragma once

#include <string>

#define TRANSLATED_GROUP_INFO(GROUP_NAME, MODULE_NAME) \
	static FeatureGUIInfo info {					   \
		GROUP_NAME,									   \
		{},									           \
													   \
		MODULE_NAME,								   \
		{}										       \
	};												   \
													   \
	info.groupRepr = _TR(GROUP_NAME);				   \
	info.moduleRepr = _TR(MODULE_NAME);				   

#define TRANSLATED_MODULE_INFO(MODULE_NAME)\
	static FeatureGUIInfo info {		   \
		{},								   \
		{},						           \
										   \
		MODULE_NAME,					   \
		{}							       \
	};									   \
										   \
	info.moduleRepr = _TR(MODULE_NAME);		


namespace cheat 
{

	struct FeatureGUIInfo
	{
		std::string groupKey;
		std::string groupRepr;

		std::string moduleKey;
		std::string moduleRepr;
	};

	class Feature
	{
	public:
		// GUI handlers
		virtual const FeatureGUIInfo& GetGUIInfo() const = 0;
		
		virtual void DrawMain() = 0;

		virtual bool NeedStatusDraw() const { return false; };
		virtual void DrawStatus() { };
		
		virtual bool NeedInfoDraw() const { return false; };
		virtual void DrawInfo() { };

		virtual void DrawExternal() { };

	protected:
		virtual ~Feature() = default;
	};
}


