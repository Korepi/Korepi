#pragma once

#include <memory>
#include "eventhandlerptr.h"


namespace events::handlers 
{
	template<class TSome>
	struct HandlerCast
	{
		template<class ...Types>
		static constexpr TEventHandlerPtr<Types...> cast(TSome& some)
		{
			return static_cast<TEventHandlerPtr<Types...>>(some);
		}
	};

	template<class TPtr>
	struct HandlerCast<std::shared_ptr<TPtr>>
	{
		template<class ...Types>
		static constexpr TEventHandlerPtr<Types...> cast(std::shared_ptr<TPtr> some)
		{
			return HandlerCast<TPtr>::template cast<Types ...>(*some);
		}
	};
}
