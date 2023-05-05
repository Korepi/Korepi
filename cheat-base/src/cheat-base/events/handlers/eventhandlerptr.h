#pragma once

#include <memory>


namespace events::handlers 
{
	template<class ...TParams> class AbstractEventHandler;

	template<class ...Types>
	using TEventHandlerPtr = std::shared_ptr<AbstractEventHandler<Types...>>;
}
