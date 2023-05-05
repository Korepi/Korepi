#pragma once

#include <map>
#include <mutex>

#include "Entity.h"
#include "IEntityFilter.h"

#include <cheat-base/events/event.hpp>

namespace cheat::game
{
	class EntityManager
	{
	public:

		EntityManager(EntityManager const&) = delete;
		void operator=(EntityManager const&) = delete;

		static EntityManager& instance();

		std::vector<app::BaseEntity*> rawEntities();
		std::vector<Entity*> entities();
		std::vector<Entity*> entities(const IEntityFilter& filter);

		typedef bool (*Validator)(game::Entity* entity);
		std::vector<Entity*> entities(Validator validator);

		Entity* entity(app::BaseEntity*);
		Entity* entity(uint32_t runtimeID, bool unsafe = false);

		Entity* avatar();

		app::CameraEntity* mainCamera();

		void OnRawEntityDestroy(app::BaseEntity* entity);

		TEvent<Entity*> entityDestroyEvent;

	private:
		EntityManager();

		std::mutex m_EntityCacheLock;
		std::map<app::BaseEntity*, std::pair<Entity*, uint32_t>> m_EntityCache;

		Entity m_AvatarEntity;

		inline static Entity* s_EmptyEntity = new Entity(nullptr);
	};
}
