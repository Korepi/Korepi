#pragma once

#include <mutex>
#include <map>

#include "Entity.h"
#include "IEntityFilter.h"

namespace cheat::game
{
	class CacheFilterExecutor
	{
	public:
		CacheFilterExecutor(uint32_t lifeTime = 1000);
		bool ApplyFilter(Entity* entity, IEntityFilter* filter);
	
	private:

		typedef std::pair<bool, uint64_t> CacheValue;

		void OnEntityDestroy(Entity* entity);

		uint32_t m_LifeTime;

		std::mutex m_CacheLock;
		std::unordered_map<Entity*, std::unordered_map<IEntityFilter*, CacheValue>> m_Cache;
	};
}
