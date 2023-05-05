#pragma once

#include "handlereventjoin.h"
#include "../event.hpp"


namespace events::joins 
{
	template<class ...TParams>
	bool HandlerEventJoin<TParams...>::isJoined() const
	{
		return m_event.isHandlerAdded(m_handler);
	}

	template<class ...TParams>
	bool HandlerEventJoin<TParams...>::join()
	{
		return m_event.addHandler(m_handler);
	}

	template<class ...TParams>
	bool HandlerEventJoin<TParams...>::unjoin()
	{
		return m_event.removeHandler(m_handler);
	}
}
