#pragma once

#include "IEntityFilter.h"
#include <vector>
#include <il2cpp-appdata.h>

namespace cheat::game
{
    class SimpleFilter :
        public IEntityFilter
    {
	public:
        SimpleFilter(app::EntityType__Enum_1 type, std::initializer_list<std::string> names = {}) : m_Type(type), m_Names{ names } {}
        SimpleFilter(app::EntityType__Enum_1 type, const std::string& name) : SimpleFilter(type, {name}) {}
        SimpleFilter(std::initializer_list<SimpleFilter> lst);
        bool IsValid(Entity* entity) const override;
    protected:
		app::EntityType__Enum_1 m_Type;
        std::vector<std::string> m_Names;
    };
}
