#pragma once

#include <vector>
#include <cheat/game/IEntityFilter.h>
#include <il2cpp-appdata.h>
#include <cheat-base/config/config.h>
#include <cheat/game/SimpleFilter.h>

namespace cheat::feature
{
    class ESPCustomFilter : public game::IEntityFilter
    {
    public:
        std::string m_FilterName;
        app::EntityType__Enum_1 m_Type;
        std::vector<std::string> m_Names;

        ESPCustomFilter() : ESPCustomFilter("", app::EntityType__Enum_1::None, {}) { }
        ESPCustomFilter(std::string filterName, app::EntityType__Enum_1 type, std::vector<std::string> names)
        {
            m_FilterName = filterName;
            m_Type = type;
            m_Names = names;
        }

        bool IsValid(game::Entity* entity) const override;

        inline app::EntityType__Enum_1 GetType() const
        {
            return m_Type;
        }

        inline void SetType(app::EntityType__Enum_1 value)
        {
            m_Type = value;
        }

        inline std::vector<std::string> GetNames() const
        {
            return m_Names;
        }

        inline void SetNames(std::vector<std::string> value)
        {
            m_Names = value;
        }
    };
}

namespace config::converters
{
    template<>
    inline nlohmann::json ToJson(const cheat::feature::ESPCustomFilter& value)
    {
        return {
            { "filter_name", value.m_FilterName },
            { "type", ToJson(config::Enum<app::EntityType__Enum_1>(value.GetType())) },
            { "names", value.GetNames() }
        };
    }

    template<>
    inline void FromJson(cheat::feature::ESPCustomFilter& value, const nlohmann::json& jObject)
    {
        value.m_FilterName = jObject["filter_name"];

        config::Enum<app::EntityType__Enum_1> type = app::EntityType__Enum_1::None;
        FromJson(type, jObject["type"]);

        value.SetType(type);
        value.SetNames(jObject["names"]);
    }
}