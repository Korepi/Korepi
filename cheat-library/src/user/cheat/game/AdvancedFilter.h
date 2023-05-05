#pragma once

#include "IEntityFilter.h"
#include <vector>
#include <il2cpp-appdata.h>

namespace cheat::game
{
    namespace filter::mode {
        inline bool whitelist(const std::vector<std::string>& names, cheat::game::Entity* entity) {
            const auto& name = entity->name();
            bool name_found = std::any_of(names.cbegin(), names.cend(),
                [&name](const std::string& pattern) { return name.find(pattern) != std::string::npos; });

            return name_found;
        }

        inline bool blacklist(const std::vector<std::string>& names, cheat::game::Entity* entity) {
            bool name_not_found = !whitelist(names, entity);
            return name_not_found;
        }
    }

    template <auto nameFilterFn = &filter::mode::whitelist>
    class AdvancedFilter :
        public IEntityFilter
    {
    public:
        AdvancedFilter(std::initializer_list<app::EntityType__Enum_1> types = {}, std::initializer_list<std::string> names = {}) : m_Types(types), m_Names(names) {}
        AdvancedFilter(std::initializer_list<app::EntityType__Enum_1> types, const std::string& name) : AdvancedFilter(types, { name }) {}
        AdvancedFilter(const app::EntityType__Enum_1 type, std::initializer_list<std::string> name) : AdvancedFilter({ type }, name) {}
        AdvancedFilter(const app::EntityType__Enum_1 type, const std::string& name) : AdvancedFilter({ type }, { name }) {}
        friend AdvancedFilter operator+(AdvancedFilter lFilter, const AdvancedFilter& rFilter);
        
        bool IsValid(Entity* entity) const override {
            if (entity == nullptr)
                return false;

            bool type_found = m_Types.size() == 0 || std::any_of(m_Types.cbegin(), m_Types.cend(),
                [entity](const app::EntityType__Enum_1& type) { return entity->type() == type; });

            if (!type_found)
                return false;

            return m_Names.empty() || nameFilterFn(m_Names, entity);
        }

    private:
        std::vector<app::EntityType__Enum_1> m_Types;
        std::vector<std::string> m_Names;
    };


    template<auto Fn>
    AdvancedFilter<Fn> operator+(AdvancedFilter<Fn> lFilter, const AdvancedFilter<Fn>& rFilter)
    {
        lFilter.m_Names.insert(lFilter.m_Names.end(), rFilter.m_Names.begin(), rFilter.m_Names.end());
        lFilter.m_Types.insert(lFilter.m_Types.end(), rFilter.m_Types.begin(), rFilter.m_Types.end());
        return lFilter;
    }

    using WhitelistFilter = AdvancedFilter<&filter::mode::whitelist>;
    using BlacklistFilter = AdvancedFilter<&filter::mode::blacklist>;
}
