#pragma once

#include "abstracteventjoin.h"
#include "../handlers/eventhandlerptr.h"


template<class ...TParams> class IEvent;

namespace events::joins 
{
	template<class ...TParams>
	class HandlerEventJoin : public AbstractEventJoin
	{
	public:

		HandlerEventJoin(IEvent<TParams...>& _event, ::events::handlers::TEventHandlerPtr<TParams...> handler) :
			AbstractEventJoin(),
			m_event(_event),
			m_handler(handler)
		{ }

		virtual inline bool isJoined() const override;
		virtual inline bool join() override;
		virtual inline bool unjoin() override;

	private:

		IEvent<TParams...>& m_event;
		::events::handlers::TEventHandlerPtr<TParams...> m_handler;
	};
}
